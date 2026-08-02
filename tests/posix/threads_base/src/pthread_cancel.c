/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <threads.h>
#include <sched.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/signal.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

#define N_THR_T 3

static void *thread_top_term(void *p1)
{
	int ret;
	int id = POINTER_TO_INT(p1);
	pthread_t self = pthread_self();

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	int policy;
	struct sched_param param = {
		.sched_priority = N_THR_T - id,
	};
	struct sched_param getschedparam;

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* Change priority of thread */
		ret = pthread_setschedparam(self, SCHED_RR, &param);
		zassert_ok(ret, "Unable to set thread priority! %d", ret);

		ret = pthread_getschedparam(self, &policy, &getschedparam);
		zassert_ok(ret, "Unable to get thread priority! %d", ret);

		printk("Thread %d starting with a priority of %d\n", id,
		       getschedparam.sched_priority);
	}
#endif

	if (!k_is_user_context()) {
		/* kernel threads must explicitly unmask cancel (not a POSIX sigaddset signo) */
		zassert_ok(k_sig_addset(&k_current_get()->base.sig.mask, K_SIG_CANCEL));
	}

	if (id % 2) {
		ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		zassert_false(ret, "Unable to set cancel state!");
	}

	if ((id % 2) == 0) {
		printk("Cancelling thread %d\n", id);
		zassert_ok(pthread_cancel(self), "Thread %d could not be cancelled\n", id);
	}
	/*
	 * ISO C, so no POSIX Option Group is required and it is safe on host
	 * libc threads. Unlike sleep(), thrd_sleep() is not a cancellation
	 * point, so cancelled threads sleep the full duration; keep it short.
	 */
	thrd_sleep(&(struct timespec){.tv_nsec = 100 * NSEC_PER_MSEC}, NULL);
	printk("Exiting thread %d\n", id);
	pthread_exit(p1);
	return NULL;
}

static void test_pthread_cancel(void)
{
	if (IS_ENABLED(CONFIG_COVERAGE)) {
		/* Coverage data increases binary size, reducing heap for dynamic stacks */
		ztest_test_skip();
	}
	int32_t i;
	pthread_t newthread[N_THR_T] = {0};
	void *retval;

	for (i = 0; i < N_THR_T; i++) {
		zassert_ok(pthread_create(&newthread[i], NULL, thread_top_term, INT_TO_POINTER(i)));
	}

	for (i = 0; i < N_THR_T; i++) {
		zassert_ok(pthread_join(newthread[i], &retval));
	}

	/*
	 * Cannot guarantee that an implementation will return ESRCH for attempting to
	 * cancel a terminated thread.
	 *
	 * Please see Rationale here:
	 * https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_cancel.html
	 */
}

ZTEST_THREADS_BASE(test_pthread_cancel);
