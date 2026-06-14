/*
 * Copyright (c) 2018-2023 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#define DETACH_THR_ID 2

#define N_THR_E    3
#define N_THR_T    3
#define BOUNCES    64
#define ONE_SECOND 1

#define PTHREAD_CANCEL_INVALID -1
#define PTHREAD_INVALID        ((pthread_t)(-1))

static void *thread_top_exec(void *p1);
static void *thread_top_term(void *p1);

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cvar0 = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cvar1 = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cvar_done = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cvar_first_iter = PTHREAD_COND_INITIALIZER;

#define N_FIRST_ITER_PEERS (N_THR_E - 1)
static int first_iter_peer_count;

static int bounce_failed;
static int bounce_done[N_THR_E];

static int curr_bounce_thread;

/* Bounces execution between threads using a condition variable, continuously
 * testing that no other thread is mucking with the protected state.
 * Test success is signaled to main() using a condition variable.
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
				pthread_cond_signal(&cvar_done);
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
	pthread_cond_signal(&cvar_done);
	pthread_cond_wait(&cvar1, &lock);
	pthread_mutex_unlock(&lock);

	printk("Thread %d done\n", id);

	pthread_exit(p1);

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

ZTEST(pthread, test_pthread_execution)
{
	if (IS_ENABLED(CONFIG_COVERAGE)) {
		/* Coverage data increases binary size, reducing heap for dynamic stacks */
		ztest_test_skip();
	}
	int i, ret;
	pthread_t newthread[N_THR_E];
	void *retval;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		/* Most of this testsuite uses automatic stack allocation, but
		 * threads_base.static_stack uses statically allocated stacks.
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
	if (cvar_done == PTHREAD_COND_INITIALIZER) {
		zassert_ok(pthread_cond_init(&cvar_done, NULL));
	}

	first_iter_peer_count = 0;
	bounce_failed = 0;
	curr_bounce_thread = 0;
	for (i = 0; i < N_THR_E; i++) {
		bounce_done[i] = 0;
	}

	for (i = 0; i < N_THR_E; i++) {
		ret = pthread_create(&newthread[i], NULL, thread_top_exec, INT_TO_POINTER(i));
		zassert_ok(ret, "pthread_create failed for thread %d: %d", i, ret);
	}

	while (!bounce_test_done()) {
		pthread_mutex_lock(&lock);
		while (!bounce_test_done()) {
			zassert_ok(pthread_cond_wait(&cvar_done, &lock));
		}
		pthread_mutex_unlock(&lock);
	}

	/* TESTPOINT: Check if bounce test passes */
	zassert_false(bounce_failed, "Bounce test failed");

	printk("Bounce test OK\n");

	/* Wake up the worker threads so they can exit */
	pthread_mutex_lock(&lock);
	pthread_cond_broadcast(&cvar1);
	pthread_mutex_unlock(&lock);

	for (i = 0; i < N_THR_E; i++) {
		zassert_ok(pthread_join(newthread[i], &retval));
	}

	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cvar0);
	pthread_cond_destroy(&cvar1);
	pthread_cond_destroy(&cvar_done);
	pthread_cond_destroy(&cvar_first_iter);
}

ZTEST(pthread, test_pthread_termination)
{
	if (IS_ENABLED(CONFIG_COVERAGE)) {
		/* Coverage data increases binary size, reducing heap for dynamic stacks */
		ztest_test_skip();
	}
	int32_t i, ret;
	pthread_t newthread[N_THR_T] = {0};
	void *retval;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		/* Most of this testsuite uses automatic stack allocation, but
		 * threads_base.static_stack uses statically allocated stacks.
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

static void *create_thread1(void *p1)
{
	ARG_UNUSED(p1);
	return NULL;
}

ZTEST(pthread, test_pthread_descriptor_leak)
{
	pthread_t pthread1;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		/* Most of this testsuite uses automatic stack allocation, but
		 * threads_base.static_stack uses statically allocated stacks.
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
		 * threads_base.static_stack uses statically allocated stacks.
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

	/* queue cancellation while disabled */
	zassert_ok(pthread_cancel(pthread_self()));

	/* cancellation point with no pending delivery while disabled */
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
		 * threads_base.static_stack uses statically allocated stacks.
		 */
		ztest_test_skip();
	}

	zassert_ok(pthread_create(&th, NULL, test_pthread_cancel_fn, NULL));
	zassert_ok(pthread_join(th, NULL));
	zassert_true(testcancel_ignored);
	zassert_false(testcancel_failed);
}

ZTEST_SUITE(pthread, NULL, NULL, NULL, NULL, NULL);
