/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <sched.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

#define N_THR_E 3
#define BOUNCES 64

static ZTEST_BMEM pthread_mutex_t lock;
static ZTEST_BMEM pthread_cond_t cvar0;
static ZTEST_BMEM pthread_cond_t cvar1;
static ZTEST_BMEM pthread_cond_t cvar_done;
static ZTEST_BMEM pthread_cond_t cvar_first_iter;

#define N_FIRST_ITER_PEERS (N_THR_E - 1)
static ZTEST_BMEM int first_iter_peer_count;

static ZTEST_BMEM int bounce_failed;
static ZTEST_BMEM int bounce_done[N_THR_E];

static ZTEST_BMEM int curr_bounce_thread;

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
					zassert_equal(0, pthread_cond_wait(&cvar_first_iter, &lock),
						      "");
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

static void pthread_sync_init(void)
{
	zassert_ok(pthread_mutex_init(&lock, NULL));
	zassert_ok(pthread_cond_init(&cvar0, NULL));
	zassert_ok(pthread_cond_init(&cvar1, NULL));
	zassert_ok(pthread_cond_init(&cvar_first_iter, NULL));
	zassert_ok(pthread_cond_init(&cvar_done, NULL));
}

static void pthread_sync_fini(void)
{
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cvar0);
	pthread_cond_destroy(&cvar1);
	pthread_cond_destroy(&cvar_done);
	pthread_cond_destroy(&cvar_first_iter);
}

static void cond_signal_bounce(void)
{
	int i, ret;
	pthread_t newthread[N_THR_E];
	void *retval;

	pthread_sync_init();

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

	pthread_sync_fini();
}

static void cond_signal_static_init(void)
{
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	zassert_ok(pthread_cond_signal(&cond));
	zassert_ok(pthread_cond_destroy(&cond));
}

static void test_pthread_cond_signal(void)
{
	cond_signal_static_init();

	if (!IS_ENABLED(CONFIG_COVERAGE)) {
		/* Coverage data increases binary size, reducing heap for dynamic stacks */
		cond_signal_bounce();
	}
}

ZTEST_THREADS_BASE(test_pthread_cond_signal);
