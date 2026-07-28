/*
 * Copyright (c) 2025 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include <zephyr/sys/timeutil.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "../../common/linux_compat_test.h"

static struct timespec sleep_timeout_abstime;

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

ZTEST(posix_non_portable, test_pthread_getname_np)
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

ZTEST(posix_non_portable, test_pthread_setname_np)
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

ZTEST(posix_non_portable, test_pthread_tryjoin_np)
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

ZTEST(posix_non_portable, test_pthread_timedjoin_np)
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
