/*
 * Copyright (c) 2025 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <zephyr/sys/timeutil.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

static ZTEST_BMEM struct timespec sleep_timeout_abstime;
static ZTEST_BMEM volatile bool affinity_stop;
static ZTEST_BMEM volatile int affinity_cpu;

static inline void timespec_add_ms(struct timespec *ts, uint32_t ms)
{
	struct timespec addend;

	timespec_from_timeout(K_MSEC(ms), &addend);
	timespec_add(ts, &addend);
}

static void *timedjoin_thread(void *p1)
{
	ARG_UNUSED(p1);

	clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &sleep_timeout_abstime, NULL);
	return NULL;
}

/* placement can only be observed from kernel mode on the Zephyr scheduler */
static bool affinity_observable(void)
{
	return !IS_ENABLED(CONFIG_NATIVE_LIBC) && !k_is_user_context();
}

static int current_cpu_id(void)
{
	unsigned int key = arch_irq_lock();
	int id = _current_cpu->id;

	arch_irq_unlock(key);

	return id;
}

/* sleeps between samples so that simulated time keeps moving on a single CPU */
static void *affinity_thread(void *p1)
{
	const struct timespec delay = {.tv_nsec = NSEC_PER_MSEC};

	ARG_UNUSED(p1);

	while (!affinity_stop) {
		if (affinity_observable()) {
			affinity_cpu = current_cpu_id();
		}
		clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
	}

	return NULL;
}

static bool wait_for_cpu(int cpu)
{
	const struct timespec delay = {.tv_nsec = 10 * NSEC_PER_MSEC};

	for (int i = 0; (i < 200) && (affinity_cpu != cpu); ++i) {
		clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
	}

	return affinity_cpu == cpu;
}

static void pthread_getaffinity_np_invalid(void)
{
	cpu_set_t set;
	pthread_t self = pthread_self();

	IF_NOT_NATIVE_LIBC({
		zassert_equal(pthread_getaffinity_np(self, sizeof(set), NULL), EFAULT);
	})
	zassert_equal(pthread_getaffinity_np(self, 0, &set), EINVAL);
	zassert_equal(pthread_getaffinity_np(self, sizeof(set) - 1, &set), EINVAL);
}

static void pthread_getaffinity_np_self(void)
{
	pthread_t self = pthread_self();
	struct {
		cpu_set_t set;
		uint32_t guard;
	} big;

	zassert_ok(pthread_getaffinity_np(self, sizeof(big.set), &big.set));
	zassert_true(CPU_COUNT(&big.set) > 0);
	IF_NOT_NATIVE_LIBC({
		zassert_equal(CPU_COUNT(&big.set), CONFIG_MP_MAX_NUM_CPUS);
		for (int cpu = 0; cpu < CONFIG_MP_MAX_NUM_CPUS; ++cpu) {
			zassert_true(CPU_ISSET(cpu, &big.set), "CPU %d missing", cpu);
		}
	})

	/* a larger buffer is accepted and cleared past the set */
	memset(&big, 0xff, sizeof(big));
	zassert_ok(pthread_getaffinity_np(self, sizeof(big), &big.set));
	zassert_true(CPU_COUNT(&big.set) > 0);
	zassert_equal(big.guard, 0);
}

static void pthread_getaffinity_np_other(void)
{
	pthread_t th;
	cpu_set_t set;
	cpu_set_t self_set;

	clock_gettime(CLOCK_REALTIME, &sleep_timeout_abstime);
	timespec_add_ms(&sleep_timeout_abstime, 200);
	zassert_ok(pthread_create(&th, NULL, timedjoin_thread, NULL));

	zassert_ok(pthread_getaffinity_np(pthread_self(), sizeof(self_set), &self_set));
	zassert_ok(pthread_getaffinity_np(th, sizeof(set), &set));
	zassert_true(CPU_EQUAL(&set, &self_set));

	zassert_ok(pthread_join(th, NULL));
}

ZTEST_USER(posix_non_portable, test_pthread_getaffinity_np)
{
	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		ztest_test_skip();
	}

	pthread_getaffinity_np_invalid();
	pthread_getaffinity_np_self();
	pthread_getaffinity_np_other();
}

static void pthread_setaffinity_np_invalid(void)
{
	cpu_set_t set;
	pthread_t self = pthread_self();

	CPU_ZERO(&set);
	CPU_SET(0, &set);
	IF_NOT_NATIVE_LIBC({
		zassert_equal(pthread_setaffinity_np(self, sizeof(set), NULL), EFAULT);
	})
	zassert_equal(pthread_setaffinity_np(self, 0, &set), EINVAL);

	/* no CPU at all, then only a CPU that does not exist */
	CPU_ZERO(&set);
	zassert_equal(pthread_setaffinity_np(self, sizeof(set), &set), EINVAL);
	CPU_SET(CPU_SETSIZE - 1, &set);
	zassert_equal(pthread_setaffinity_np(self, sizeof(set), &set), EINVAL);
}

static void pthread_setaffinity_np_self(void)
{
	cpu_set_t set;
	cpu_set_t initial;
	pthread_t self = pthread_self();

	zassert_ok(pthread_getaffinity_np(self, sizeof(initial), &initial));

	for (int cpu = 0; cpu < CPU_SETSIZE; ++cpu) {
		if (!CPU_ISSET(cpu, &initial)) {
			continue;
		}

		CPU_ZERO(&set);
		CPU_SET(cpu, &set);
		zassert_ok(pthread_setaffinity_np(self, sizeof(set), &set));
		zassert_ok(pthread_getaffinity_np(self, sizeof(set), &set));
		zassert_equal(CPU_COUNT(&set), 1);
		zassert_true(CPU_ISSET(cpu, &set));
		if (affinity_observable()) {
			zassert_equal(current_cpu_id(), cpu, "not running on CPU %d", cpu);
		}
	}

	/* CPUs that do not exist are ignored when a real one remains */
	set = initial;
	CPU_SET(CPU_SETSIZE - 1, &set);
	zassert_ok(pthread_setaffinity_np(self, sizeof(set), &set));
	zassert_ok(pthread_getaffinity_np(self, sizeof(set), &set));
	zassert_true(CPU_EQUAL(&set, &initial));
}

static void pthread_setaffinity_np_other(void)
{
	pthread_t th;
	cpu_set_t set;
	cpu_set_t initial;

	affinity_stop = false;
	affinity_cpu = -1;
	zassert_ok(pthread_create(&th, NULL, affinity_thread, NULL));
	zassert_ok(pthread_getaffinity_np(th, sizeof(initial), &initial));

	/* the target keeps running while it is moved from CPU to CPU */
	for (int cpu = 0; cpu < CPU_SETSIZE; ++cpu) {
		if (!CPU_ISSET(cpu, &initial)) {
			continue;
		}

		CPU_ZERO(&set);
		CPU_SET(cpu, &set);
		zassert_ok(pthread_setaffinity_np(th, sizeof(set), &set));
		zassert_ok(pthread_getaffinity_np(th, sizeof(set), &set));
		zassert_equal(CPU_COUNT(&set), 1);
		zassert_true(CPU_ISSET(cpu, &set));
		if (affinity_observable()) {
			zassert_true(wait_for_cpu(cpu), "thread did not move to CPU %d", cpu);
		}
	}

	zassert_ok(pthread_setaffinity_np(th, sizeof(initial), &initial));
	affinity_stop = true;
	zassert_ok(pthread_join(th, NULL));
}

static void pthread_setaffinity_np_unsupported(void)
{
	cpu_set_t set;
	cpu_set_t initial;
	pthread_t self = pthread_self();

	zassert_ok(pthread_getaffinity_np(self, sizeof(initial), &initial));
	zassert_ok(pthread_setaffinity_np(self, sizeof(initial), &initial));

	if (CPU_COUNT(&initial) > 1) {
		CPU_ZERO(&set);
		CPU_SET(0, &set);
		zassert_equal(pthread_setaffinity_np(self, sizeof(set), &set), ENOTSUP);
	}
}

ZTEST_USER(posix_non_portable, test_pthread_setaffinity_np)
{
	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		ztest_test_skip();
	}

	pthread_setaffinity_np_invalid();
	if (IS_ENABLED(CONFIG_NATIVE_LIBC) || IS_ENABLED(CONFIG_SCHED_CPU_MASK)) {
		pthread_setaffinity_np_self();
		pthread_setaffinity_np_other();
	} else {
		pthread_setaffinity_np_unsupported();
	}
}

ZTEST_USER(posix_non_portable, test_pthread_getname_np)
{
	pthread_t th;
	static const char thr_name[] = "thread name";
	char thr_name_buf[CONFIG_THREAD_MAX_NAME_LEN];

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		ztest_test_skip();
	}

	/* keep the thread alive so that the name can be set and read back */
	clock_gettime(CLOCK_REALTIME, &sleep_timeout_abstime);
	timespec_add_ms(&sleep_timeout_abstime, 200);

	zassert_ok(pthread_create(&th, NULL, timedjoin_thread, NULL));

#if !defined(CONFIG_NATIVE_LIBC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull"
	zassert_equal(pthread_getname_np(th, NULL, sizeof(thr_name_buf)), EFAULT);
#pragma GCC diagnostic pop
#endif

	zassert_ok(pthread_setname_np(th, thr_name));
	zassert_equal(pthread_getname_np(th, thr_name_buf, strlen(thr_name) / 2), ERANGE);
	zassert_ok(pthread_getname_np(th, thr_name_buf, sizeof(thr_name_buf)));
	zassert_ok(strncmp(thr_name, thr_name_buf, MIN(strlen(thr_name), strlen(thr_name_buf))));

	zassert_ok(pthread_join(th, NULL));
}

ZTEST_USER(posix_non_portable, test_pthread_setname_np)
{
	pthread_t th;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		ztest_test_skip();
	}

	/* keep the thread alive so that the name can be set */
	clock_gettime(CLOCK_REALTIME, &sleep_timeout_abstime);
	timespec_add_ms(&sleep_timeout_abstime, 200);

	zassert_ok(pthread_create(&th, NULL, timedjoin_thread, NULL));
	zassert_ok(pthread_setname_np(th, "np_setname"));
	zassert_ok(pthread_join(th, NULL));
}

ZTEST_USER(posix_non_portable, test_pthread_tryjoin_np)
{
	pthread_t th = {0};
	int sleep_duration_ms = 200;
	void *retval;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		ztest_test_skip();
	}

	clock_gettime(CLOCK_REALTIME, &sleep_timeout_abstime);
	timespec_add_ms(&sleep_timeout_abstime, sleep_duration_ms);

	zassert_ok(pthread_create(&th, NULL, timedjoin_thread, NULL));

	usleep(USEC_PER_MSEC * sleep_duration_ms / 2);
	zassert_equal(pthread_tryjoin_np(th, &retval), EBUSY);

	usleep(USEC_PER_MSEC * sleep_duration_ms);
	zassert_ok(pthread_tryjoin_np(th, &retval));
}

ZTEST_USER(posix_non_portable, test_pthread_timedjoin_np)
{
	int ret;
	void *result;
	pthread_t th = {0};
	struct timespec done;
	struct timespec not_done;
	int sleep_duration_ms = 200;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		ztest_test_skip();
	}

	/* the host libc does not validate the timespec of an unstarted (zeroed) thread id */
	IF_NOT_NATIVE_LIBC({
		struct timespec invalid[] = {
			{.tv_nsec = -1},
			{.tv_nsec = NSEC_PER_SEC},
		};

		for (size_t i = 0; i < ARRAY_SIZE(invalid); ++i) {
			zassert_equal(pthread_timedjoin_np(th, &result, &invalid[i]), EINVAL);
		}
	})

	clock_gettime(CLOCK_REALTIME, &sleep_timeout_abstime);
	done = not_done = sleep_timeout_abstime;
	timespec_add_ms(&sleep_timeout_abstime, sleep_duration_ms);
	timespec_add_ms(&done, 2 * sleep_duration_ms);
	timespec_add_ms(&not_done, sleep_duration_ms / 2);

	zassert_ok(pthread_create(&th, NULL, timedjoin_thread, NULL));

	ret = pthread_timedjoin_np(th, &result, &not_done);
	zassert_equal(ret, ETIMEDOUT, "pthread_timedjoin_np failed with error %d", ret);

	/* an already-past absolute deadline times out without blocking */
	IF_NOT_NATIVE_LIBC({
		const struct timespec past = {0};
		const int64_t start = k_uptime_get();

		ret = pthread_timedjoin_np(th, &result, &past);
		zassert_equal(ret, ETIMEDOUT, "pthread_timedjoin_np failed with error %d", ret);
		zassert_true(k_uptime_get() - start < sleep_duration_ms / 2,
			     "past deadline blocked");
	})

	ret = pthread_timedjoin_np(th, &result, &done);
	zassert_ok(ret, "pthread_timedjoin_np failed with error %d", ret);

	/* as on Linux, a NULL abstime blocks indefinitely, like pthread_join() */
	IF_NOT_NATIVE_LIBC({
		zassert_ok(pthread_create(&th, NULL, timedjoin_thread, NULL));
		ret = pthread_timedjoin_np(th, &result, NULL);
		zassert_ok(ret, "pthread_timedjoin_np failed with error %d", ret);
	})
}

ZTEST_SUITE(posix_non_portable, NULL, NULL, NULL, NULL, NULL);
