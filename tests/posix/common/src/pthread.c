/*
 * Copyright (c) 2018-2023 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <zephyr/sys/util.h>
#include <zephyr/sys/timeutil.h>
#include <zephyr/ztest.h>

#define DETACH_THR_ID 2

#define N_THR_E    3
#define N_THR_T    3
#define BOUNCES    64
#define ONE_SECOND 1

/* Macros to test invalid states */
#define PTHREAD_CANCEL_INVALID -1
#define SCHED_INVALID          -1
#define PRIO_INVALID           -1
#define PTHREAD_INVALID        ((pthread_t)(-1))

/* TODO: move tests that use non-portable calls to an "np" testsuite */
int pthread_setname_np(pthread_t thread, const char *name);
int pthread_getname_np(pthread_t thread, char *name, size_t len);
int pthread_tryjoin_np(pthread_t thread, void **retval);
int pthread_timedjoin_np(pthread_t thread, void **retval, const struct timespec *abstime);

static void *thread_top_exec(void *p1);
static void *thread_top_term(void *p1);

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cvar0 = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cvar1 = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cvar_first_iter = PTHREAD_COND_INITIALIZER;
static pthread_barrier_t barrier;

#define N_FIRST_ITER_PEERS (N_THR_E - 1)
static int first_iter_peer_count;

static sem_t main_sem;

static int bounce_failed;
static int bounce_done[N_THR_E];

static int curr_bounce_thread;

static int barrier_failed;
static int barrier_done[N_THR_E];
static int barrier_return[N_THR_E];

static struct timespec sleep_timeout_abstime;

static inline void timespec_add_ms(struct timespec *ts, uint32_t ms)
{
	struct timespec addend;

	timespec_from_timeout(K_MSEC(ms), &addend);
	timespec_add(ts, &addend);
}

/* First phase bounces execution between threads using a condition
 * variable, continuously testing that no other thread is mucking with
 * the protected state.  The first iteration uses an explicit handshake
 * (not timing) so peer threads block on cvar0 before thread 0 signals.
 * This ends with all threads going back to sleep on the condition variable
 * and being woken by main() for the second phase.
 *
 * Second phase simply lines up all the threads on a barrier, verifies
 * that none run until the last one enters, and that all run after the
 * exit.
 *
 * Test success is signaled to main() using a traditional semaphore.
 */

static void *thread_top_exec(void *p1)
{
	int i, j, id = (int)POINTER_TO_INT(p1);

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	int policy;
	struct sched_param schedparam;

	pthread_getschedparam(pthread_self(), &policy, &schedparam);
	printk("Thread %d starting with scheduling policy %d & priority %d\n", id, policy,
	       schedparam.sched_priority);
#endif

	/* Try a double-lock here to exercise the failing case of
	 * trylock.  We don't support RECURSIVE locks, so this is
	 * guaranteed to fail.
	 */
	pthread_mutex_lock(&lock);

	if (!pthread_mutex_trylock(&lock)) {
		printk("pthread_mutex_trylock inexplicably succeeded\n");
		bounce_failed = 1;
	}

	pthread_mutex_unlock(&lock);

	for (i = 0; i < BOUNCES; i++) {

		pthread_mutex_lock(&lock);

		if (i == 0) {
			if (id == 0) {
				while (first_iter_peer_count < N_FIRST_ITER_PEERS) {
					zassert_equal(0, pthread_cond_wait(&cvar_first_iter, &lock), "");
				}
			} else {
				first_iter_peer_count++;
				zassert_equal(0, pthread_cond_signal(&cvar_first_iter), "");
				zassert_equal(0, pthread_cond_wait(&cvar0, &lock), "");
			}
		} else {
			zassert_equal(0, pthread_cond_wait(&cvar0, &lock), "");
		}

		/* Claim ownership, then try really hard to give someone
		 * else a shot at hitting this if they are racing.
		 */
		curr_bounce_thread = id;
		for (j = 0; j < 1000; j++) {
			if (curr_bounce_thread != id) {
				printk("Racing bounce threads\n");
				bounce_failed = 1;
				sem_post(&main_sem);
				pthread_mutex_unlock(&lock);
				return NULL;
			}
			sched_yield();
		}

		/* Next one's turn, go back to the top and wait.  */
		pthread_cond_signal(&cvar0);
		pthread_mutex_unlock(&lock);
	}

	/* Signal we are complete to main(), then let it wake us up.  Note
	 * that we are using the same mutex with both cvar0 and cvar1,
	 * which is non-standard but kosher per POSIX (and it works fine
	 * in our implementation
	 */
	pthread_mutex_lock(&lock);
	bounce_done[id] = 1;
	sem_post(&main_sem);
	pthread_cond_wait(&cvar1, &lock);
	pthread_mutex_unlock(&lock);

	/* Now just wait on the barrier.  Make sure no one else finished
	 * before we wait on it, then signal that we're done
	 */
	for (i = 0; i < N_THR_E; i++) {
		if (barrier_done[i]) {
			printk("Barrier exited early\n");
			barrier_failed = 1;
			sem_post(&main_sem);
		}
	}
	barrier_return[id] = pthread_barrier_wait(&barrier);
	barrier_done[id] = 1;
	sem_post(&main_sem);

	printk("Thread %d done\n", id);

	pthread_exit(p1);

	return NULL;
}

static void *timedjoin_thread(void *p1)
{
	clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &sleep_timeout_abstime, NULL);
	return NULL;
}

static int bounce_test_done(void)
{
	int i;

	if (bounce_failed) {
		return 1;
	}

	for (i = 0; i < N_THR_E; i++) {
		if (!bounce_done[i]) {
			return 0;
		}
	}

	return 1;
}

static int barrier_test_done(void)
{
	int i;

	if (barrier_failed) {
		return 1;
	}

	for (i = 0; i < N_THR_E; i++) {
		if (!barrier_done[i]) {
			return 0;
		}
	}

	return 1;
}

static void *thread_top_term(void *p1)
{
	pthread_t self;
	int ret;
	int id = POINTER_TO_INT(p1);

	self = pthread_self();

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	int policy;
	struct sched_param param = {
		.sched_priority = N_THR_T - id,
	};
	struct sched_param getschedparam;

	/* Change priority of thread */
	zassert_false(pthread_setschedparam(self, SCHED_RR, &param),
		      "Unable to set thread priority!");

	zassert_false(pthread_getschedparam(self, &policy, &getschedparam),
		      "Unable to get thread priority!");

	printk("Thread %d starting with a priority of %d\n", id, getschedparam.sched_priority);
#endif

	if (!k_is_user_context()) {
		/* kernel threads must explicitly unmask any signals they wish to receive */
		sigset_t mask;

		zassert_ok(sigemptyset(&mask));
		zassert_ok(sigaddset(&mask, K_SIG_CANCEL));
		zassert_ok(pthread_sigmask(SIG_UNBLOCK, &mask, NULL));
	}

	if (id % 2) {
		ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		zassert_false(ret, "Unable to set cancel state!");
	}

	if ((id % 2) == 0) {
		printk("Cancelling thread %d\n", id);
		zassert_ok(pthread_cancel(self), "Thread %d could not be cancelled\n", id);
	}
	sleep(ONE_SECOND);
	printk("Exiting thread %d\n", id);
	pthread_exit(p1);
	return NULL;
}

/* Test the internal priority conversion functions */
int zephyr_to_posix_priority(int z_prio, int *policy);
int posix_to_zephyr_priority(int priority, int policy);
ZTEST(pthread, test_pthread_priority_conversion)
{
	/*
	 *    ZEPHYR [-CONFIG_NUM_COOP_PRIORITIES, -1]
	 *                       TO
	 * POSIX(FIFO) [0, CONFIG_NUM_COOP_PRIORITIES - 1]
	 */
	for (int z_prio = -CONFIG_NUM_COOP_PRIORITIES, prio = CONFIG_NUM_COOP_PRIORITIES - 1,
		 p_prio, policy;
	     z_prio <= -1; z_prio++, prio--) {
		p_prio = zephyr_to_posix_priority(z_prio, &policy);
		zassert_equal(policy, SCHED_FIFO);
		zassert_equal(p_prio, prio, "%d %d\n", p_prio, prio);
		zassert_equal(z_prio, posix_to_zephyr_priority(p_prio, SCHED_FIFO));
	}

	/*
	 *  ZEPHYR [0, CONFIG_NUM_PREEMPT_PRIORITIES - 1]
	 *                      TO
	 * POSIX(RR) [0, CONFIG_NUM_PREEMPT_PRIORITIES - 1]
	 */
	for (int z_prio = 0, prio = CONFIG_NUM_PREEMPT_PRIORITIES - 1, p_prio, policy;
	     z_prio < CONFIG_NUM_PREEMPT_PRIORITIES; z_prio++, prio--) {
		p_prio = zephyr_to_posix_priority(z_prio, &policy);
		zassert_equal(policy, SCHED_RR);
		zassert_equal(p_prio, prio, "%d %d\n", p_prio, prio);
		zassert_equal(z_prio, posix_to_zephyr_priority(p_prio, SCHED_RR));
	}
}

ZTEST(pthread, test_pthread_execution)
{
	int i, ret;
	pthread_t newthread[N_THR_E];
	void *retval;
	int serial_threads = 0;
	static const char thr_name[] = "thread name";
	char thr_name_buf[CONFIG_THREAD_MAX_NAME_LEN];

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		/* Most of this testsuite uses automatic stack allocation, but
		 * portability.posix.common.static_stack is specifically for testing with statically
		 * allocated stacks. Eventually, that configuration should be removed from
		 * common/testcase.yaml as it is already covered by the xsi_threads_ext testsuite.
		 */
		ztest_test_skip();
	}

	if (lock == PTHREAD_MUTEX_INITIALIZER) {
		zassert_ok(pthread_mutex_init(&lock, NULL));
	}
	if (cvar0 == PTHREAD_COND_INITIALIZER) {
		zassert_ok(pthread_cond_init(&cvar0, NULL));
	}
	if (cvar1 == PTHREAD_COND_INITIALIZER) {
		zassert_ok(pthread_cond_init(&cvar1, NULL));
	}
	if (cvar_first_iter == PTHREAD_COND_INITIALIZER) {
		zassert_ok(pthread_cond_init(&cvar_first_iter, NULL));
	}

	/*
	 * initialize barriers the standard way after deprecating
	 * PTHREAD_BARRIER_DEFINE().
	 */
	zassert_ok(pthread_barrier_init(&barrier, NULL, N_THR_E));

	sem_init(&main_sem, 0, 1);

	first_iter_peer_count = 0;

	for (i = 0; i < N_THR_E; i++) {
		ret = pthread_create(&newthread[i], NULL, thread_top_exec, INT_TO_POINTER(i));
		zassert_ok(ret, "pthread_create failed for thread %d: %d", i, ret);
	}

	/* TESTPOINT: Try getting thread name with no buffer */
	ret = pthread_getname_np(newthread[0], NULL, sizeof(thr_name_buf));
	zassert_equal(ret, EFAULT);

	/* TESTPOINT: Try setting thread name */
	ret = pthread_setname_np(newthread[0], thr_name);
	zassert_false(ret, "Set thread name failed!");

	ret = pthread_getname_np(newthread[0], thr_name_buf, strlen(thr_name) / 2);
	zassert_equal(ret, ERANGE);

	/* TESTPOINT: Try getting thread name */
	ret = pthread_getname_np(newthread[0], thr_name_buf, sizeof(thr_name_buf));
	zassert_false(ret, "Get thread name failed!");

	/* TESTPOINT: Thread names match */
	ret = strncmp(thr_name, thr_name_buf, MIN(strlen(thr_name), strlen(thr_name_buf)));
	zassert_false(ret, "Thread names don't match!");

	while (!bounce_test_done()) {
		sem_wait(&main_sem);
	}

	/* TESTPOINT: Check if bounce test passes */
	zassert_false(bounce_failed, "Bounce test failed");

	printk("Bounce test OK\n");

	/* Wake up the worker threads */
	pthread_mutex_lock(&lock);
	pthread_cond_broadcast(&cvar1);
	pthread_mutex_unlock(&lock);

	while (!barrier_test_done()) {
		sem_wait(&main_sem);
	}

	/* TESTPOINT: Check if barrier test passes */
	zassert_false(barrier_failed, "Barrier test failed");

	for (i = 0; i < N_THR_E; i++) {
		zassert_ok(pthread_join(newthread[i], &retval));
	}

	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cvar0);
	pthread_cond_destroy(&cvar1);
	pthread_cond_destroy(&cvar_first_iter);

	for (i = 0; i < N_THR_E; i++) {
		if (barrier_return[i] == PTHREAD_BARRIER_SERIAL_THREAD) {
			++serial_threads;
		}
	}

	/* TESTPOINT: Check only one PTHREAD_BARRIER_SERIAL_THREAD returned. */
	zassert_true(serial_threads == 1, "Bungled barrier return value(s)");

	printk("Barrier test OK\n");

	zassert_ok(pthread_barrier_destroy(&barrier));
}

ZTEST(pthread, test_pthread_termination)
{
	int32_t i, ret;
	pthread_t newthread[N_THR_T] = {0};
	void *retval;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		/* Most of this testsuite uses automatic stack allocation, but
		 * portability.posix.common.static_stack is specifically for testing with statically
		 * allocated stacks. Eventually, that configuration should be removed from
		 * common/testcase.yaml as it is already covered by the xsi_threads_ext testsuite.
		 */
		ztest_test_skip();
	}

	/* Creating 4 threads */
	for (i = 0; i < N_THR_T; i++) {
		zassert_ok(pthread_create(&newthread[i], NULL, thread_top_term, INT_TO_POINTER(i)));
	}

	/* TESTPOINT: Try setting invalid cancel state to current thread */
	ret = pthread_setcancelstate(PTHREAD_CANCEL_INVALID, NULL);
	zassert_equal(ret, EINVAL, "invalid cancel state set!");

	for (i = 0; i < N_THR_T; i++) {
		zassert_ok(pthread_join(newthread[i], &retval));
	}

	/* TESTPOINT: Test for deadlock */
	ret = pthread_join(pthread_self(), &retval);
	zassert_equal(ret, EDEADLK, "thread joined with self inexplicably!");

	/* TESTPOINT: Try canceling a terminated thread */
	ret = pthread_cancel(newthread[0]);
	zassert_equal(ret, ESRCH, "cancelled a terminated thread!");
}

ZTEST(pthread, test_pthread_tryjoin)
{
	pthread_t th = {0};
	int sleep_duration_ms = 200;
	void *retval;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		/* Most of this testsuite uses automatic stack allocation, but
		 * portability.posix.common.static_stack is specifically for testing with statically
		 * allocated stacks. Eventually, that configuration should be removed from
		 * common/testcase.yaml as it is already covered by the xsi_threads_ext testsuite.
		 */
		ztest_test_skip();
	}

	clock_gettime(CLOCK_REALTIME, &sleep_timeout_abstime);
	timespec_add_ms(&sleep_timeout_abstime, sleep_duration_ms);

	/* Creating a thread that exits after 200ms*/
	zassert_ok(pthread_create(&th, NULL, timedjoin_thread, NULL));

	/* Attempting to join, when thread is still running, should fail */
	usleep(USEC_PER_MSEC * sleep_duration_ms / 2);
	zassert_equal(pthread_tryjoin_np(th, &retval), EBUSY);

	/* Sleep so thread will exit */
	usleep(USEC_PER_MSEC * sleep_duration_ms);

	/* Attempting to join without blocking should succeed now */
	zassert_ok(pthread_tryjoin_np(th, &retval));
}

ZTEST(pthread, test_pthread_timedjoin)
{
	int ret;
	void *result;
	pthread_t th = {0};
	struct timespec done;
	struct timespec not_done;
	struct timespec invalid[] = {
		{.tv_nsec = -1},
		{.tv_nsec = NSEC_PER_SEC},
	};
	int sleep_duration_ms = 200;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		/* Most of this testsuite uses automatic stack allocation, but
		 * portability.posix.common.static_stack is specifically for testing with statically
		 * allocated stacks. Eventually, that configuration should be removed from
		 * common/testcase.yaml as it is already covered by the xsi_threads_ext testsuite.
		 */
		ztest_test_skip();
	}

	/* pthread_timedjoin_np must return EINVAL for invalid struct timespecs */
	for (size_t i = 0; i < ARRAY_SIZE(invalid); ++i) {
		zassert_equal(pthread_timedjoin_np(th, &result, &invalid[i]), EINVAL);
	}

	clock_gettime(CLOCK_REALTIME, &sleep_timeout_abstime);
	done = not_done = sleep_timeout_abstime;
	timespec_add_ms(&sleep_timeout_abstime, sleep_duration_ms);
	timespec_add_ms(&done, 2 * sleep_duration_ms);
	timespec_add_ms(&not_done, sleep_duration_ms / 2);

	zassert_ok(pthread_create(&th, NULL, timedjoin_thread, NULL));

	ret = pthread_timedjoin_np(th, &result, &not_done);
	zassert_equal(ret, ETIMEDOUT, "pthread_timedjoin_np failed with error %d", ret);

	ret = pthread_timedjoin_np(th, &result, &done);
	zassert_ok(ret, "pthread_timedjoin_np failed with error %d", ret);
}

static void *create_thread1(void *p1)
{
	/* do nothing */
	return NULL;
}

ZTEST(pthread, test_pthread_descriptor_leak)
{
	pthread_t pthread1;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		/* Most of this testsuite uses automatic stack allocation, but
		 * portability.posix.common.static_stack is specifically for testing with statically
		 * allocated stacks. Eventually, that configuration should be removed from
		 * common/testcase.yaml as it is already covered by the xsi_threads_ext testsuite.
		 */
		ztest_test_skip();
	}

	/* If we are leaking descriptors, then this loop will never complete */
	for (size_t i = 0; i < CONFIG_POSIX_THREAD_THREADS_MAX * 2; ++i) {
		zassert_ok(pthread_create(&pthread1, NULL, create_thread1, NULL),
			   "unable to create thread %zu", i);
		zassert_ok(pthread_join(pthread1, NULL), "unable to join thread %zu", i);
	}
}

ZTEST(pthread, test_pthread_equal)
{
	zassert_true(pthread_equal(pthread_self(), pthread_self()));
	zassert_false(pthread_equal(pthread_self(), (pthread_t)4242));
	zassert_true(pthread_equal(pthread_self(), (pthread_t)(uintptr_t)k_current_get()));
}

static void cleanup_handler(void *arg)
{
	bool *boolp = (bool *)arg;

	*boolp = true;
}

static void *test_pthread_cleanup_entry(void *arg)
{
	bool executed[2] = {0};

	pthread_cleanup_push(cleanup_handler, &executed[0]);
	pthread_cleanup_push(cleanup_handler, &executed[1]);
	pthread_cleanup_pop(false);
	pthread_cleanup_pop(true);

	zassert_true(executed[0]);
	zassert_false(executed[1]);

	return NULL;
}

ZTEST(pthread, test_pthread_cleanup)
{
	pthread_t th;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		/* Most of this testsuite uses automatic stack allocation, but
		 * portability.posix.common.static_stack is specifically for testing with statically
		 * allocated stacks. Eventually, that configuration should be removed from
		 * common/testcase.yaml as it is already covered by the xsi_threads_ext testsuite.
		 */
		ztest_test_skip();
	}

	zassert_ok(pthread_create(&th, NULL, test_pthread_cleanup_entry, NULL));
	zassert_ok(pthread_join(th, NULL));
}

static bool testcancel_ignored;
static bool testcancel_failed;

static void *test_pthread_cancel_fn(void *arg)
{
	if (!k_is_user_context()) {
		/* kernel threads must explicitly unmask any signals they wish to receive */
		sigset_t mask;

		zassert_ok(sigemptyset(&mask));
		zassert_ok(sigaddset(&mask, K_SIG_CANCEL));
		zassert_ok(pthread_sigmask(SIG_UNBLOCK, &mask, NULL));
	}

	zassert_ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL));

	testcancel_ignored = false;

	/* this should be ignored, but will mark cancellation as pending */
	pthread_testcancel();

	testcancel_ignored = true;

	testcancel_failed = false;

	/* enable the thread to be cancelled, the thread should not return */
	zassert_ok(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL));

	testcancel_failed = true;

	return NULL;
}

ZTEST(pthread, test_pthread_testcancel)
{
	pthread_t th;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		/* Most of this testsuite uses automatic stack allocation, but
		 * portability.posix.common.static_stack is specifically for testing with statically
		 * allocated stacks. Eventually, that configuration should be removed from
		 * common/testcase.yaml as it is already covered by the xsi_threads_ext testsuite.
		 */
		ztest_test_skip();
	}

	zassert_ok(pthread_create(&th, NULL, test_pthread_cancel_fn, NULL));
	zassert_ok(pthread_join(th, NULL));
	zassert_true(testcancel_ignored);
	zassert_false(testcancel_failed);
}

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
ZTEST(pthread, test_pthread_setschedprio)
{
	int policy;
	int prio = 0;
	struct sched_param param;
	pthread_t self = pthread_self();

	zassert_equal(pthread_setschedprio(self, PRIO_INVALID), EINVAL, "EINVAL was expected");

	zassert_ok(pthread_setschedprio(self, prio));
	param.sched_priority = ~prio;
	zassert_ok(pthread_getschedparam(self, &policy, &param));
	zassert_equal(param.sched_priority, prio, "Priority unchanged");
}
#endif

ZTEST_SUITE(pthread, NULL, NULL, NULL, NULL, NULL);
