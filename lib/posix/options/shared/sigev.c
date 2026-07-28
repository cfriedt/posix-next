/*
 * SPDX-FileCopyrightText: Copyright Friedt Professional Engineering Services, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

/* for SIGEV_THREAD_ID, where the libc provides it */
#define _GNU_SOURCE

#include "posix_internal.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#include <zephyr/kernel.h>

#ifdef CONFIG_SIGNAL
/* the queue rejects signos above SIGNAL_SET_SIZE; the internal wake signo must fit */
BUILD_ASSERT(POSIX_SIG_TIMER <= SIGNAL_SET_SIZE,
	     "SIGNAL_SET_SIZE too small for the internal timer wake signal; raise "
	     "CONFIG_SIGNAL_SET_SIZE (or lower CONFIG_POSIX_RTSIG_MAX)");
#endif

static union k_sig_val sigval_to_k(union sigval value)
{
	union k_sig_val kv;

	/* both unions overlay an int and a pointer at offset zero */
	kv.sival_ptr = value.sival_ptr;

	return kv;
}

static union sigval sigval_from_k(union k_sig_val value)
{
	union sigval v;

	v.sival_ptr = value.sival_ptr;

	return v;
}

int posix_sigev_validate(const struct sigevent *evp, bool thread_via_signal)
{
	if (evp == NULL) {
		return -EINVAL;
	}

	switch (evp->sigev_notify) {
	case SIGEV_NONE:
		return 0;
	case SIGEV_SIGNAL:
		if (!IS_ENABLED(CONFIG_SIGNAL)) {
			return -ENOTSUP;
		}
		if (z_sig_from_posix(evp->sigev_signo) < 0) {
			return -EINVAL;
		}
		return 0;
	case SIGEV_THREAD:
		if (!IS_ENABLED(CONFIG_POSIX_THREADS)) {
			return -ENOTSUP;
		}
		if (thread_via_signal && !IS_ENABLED(CONFIG_SIGNAL)) {
			return -ENOTSUP;
		}
		if (evp->sigev_notify_function == NULL) {
			return -EINVAL;
		}
		return 0;
#ifdef SIGEV_THREAD_ID
	case SIGEV_THREAD_ID:
		if (!IS_ENABLED(CONFIG_SIGNAL)) {
			return -ENOTSUP;
		}
		if (z_sig_from_posix(evp->sigev_signo) < 0) {
			return -EINVAL;
		}
		if (evp->sigev_notify_thread_id == 0) {
			return -EINVAL;
		}
		return 0;
#endif /* SIGEV_THREAD_ID */
	default:
		return -EINVAL;
	}
}

#ifdef CONFIG_POSIX_THREADS

#ifdef CONFIG_SIGNAL
static void *posix_sigev_helper(void *arg)
{
	struct k_sig_set set;
	struct k_sig_info info;
	struct posix_sigev_thread *const ctx = arg;
	void (*const fn)(union sigval) = ctx->fn;

	(void)k_sig_emptyset(&set);
	(void)k_sig_addset(&set, POSIX_SIG_TIMER);
	(void)k_sig_mask(K_SIG_BLOCK, &set, NULL);

	/*
	 * Thread permissions only inherit parent to child: grant the creator access to
	 * this thread so that timer_delete() may queue the stop wake-up signal.
	 */
	if (IS_ENABLED(CONFIG_USERSPACE)) {
		k_object_access_grant(k_current_get(), ctx->creator);
	}

	/* the wake signal is blocked: signal readiness to posix_sigev_thread_start() */
	(void)sys_sem_give(&ctx->done);

	while (!ctx->exiting) {
		if (k_sig_timedwait(&set, &info, K_FOREVER) < 0) {
			/* defensive: never let a persistent error become a busy spin */
			k_sleep(K_TICKS(1));
			continue;
		}
		if (ctx->exiting) {
			break;
		}

		fn(sigval_from_k(info.value));
	}

	/* very last touch of ctx: after this, timer_delete() may free it */
	(void)sys_sem_give(&ctx->done);

	return NULL;
}

int posix_sigev_thread_start(struct posix_sigev_thread *ctx, const struct sigevent *evp)
{
	int ret;
	pthread_attr_t attr;
	const bool have_attr = evp->sigev_notify_attributes != NULL;

	ctx->fn = evp->sigev_notify_function;
	ctx->creator = k_current_get();
	ctx->exiting = false;
	(void)sys_sem_init(&ctx->done, 0, 1);

	if (have_attr) {
		/* copy: never mutate caller-owned attributes */
		attr = *(const pthread_attr_t *)evp->sigev_notify_attributes;
	} else {
		(void)pthread_attr_init(&attr);
	}
	(void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	ret = pthread_create(&ctx->tid, &attr, posix_sigev_helper, ctx);
	if (!have_attr) {
		(void)pthread_attr_destroy(&attr);
	}
	if (ret != 0) {
		return -ret;
	}

	/* wait until the helper has blocked the wake signal: no lost-wakeup window */
	(void)sys_sem_take(&ctx->done, K_FOREVER);

	return 0;
}

void posix_sigev_thread_stop(struct posix_sigev_thread *ctx)
{
	int ret;

	ctx->exiting = true;

	/* wake the helper; a queue-full condition resolves as the helper drains signals */
	while ((ret = k_sig_queue((k_pid_t)to_k_thread(&ctx->tid), POSIX_SIG_TIMER,
				  (union k_sig_val){0})) != 0) {
		if ((ret != -EAGAIN) && (ret != -EWOULDBLOCK)) {
			/* unexpected: leak the helper rather than block forever */
			return;
		}
		k_sleep(K_MSEC(1));
	}

	/* serializes against an in-flight notification function, then ctx may be freed */
	(void)sys_sem_take(&ctx->done, K_FOREVER);
}
#endif /* CONFIG_SIGNAL */

/* one-shot SIGEV_THREAD dispatch (mq_notify) */
struct posix_sigev_oneshot {
	void (*fn)(union sigval);
	union sigval value;
};

static void *posix_sigev_oneshot_entry(void *arg)
{
	struct posix_sigev_oneshot *const ctx = arg;
	void (*const fn)(union sigval) = ctx->fn;
	const union sigval value = ctx->value;

	free(ctx);
	fn(value);

	return NULL;
}

static int posix_sigev_oneshot_start(const struct sigevent *evp)
{
	int ret;
	pthread_t tid;
	pthread_attr_t attr;
	struct posix_sigev_oneshot *ctx;
	const bool have_attr = evp->sigev_notify_attributes != NULL;

	ctx = malloc(sizeof(*ctx));
	if (ctx == NULL) {
		return -EAGAIN;
	}
	ctx->fn = evp->sigev_notify_function;
	ctx->value = evp->sigev_value;

	if (have_attr) {
		attr = *(const pthread_attr_t *)evp->sigev_notify_attributes;
	} else {
		(void)pthread_attr_init(&attr);
	}
	(void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	ret = pthread_create(&tid, &attr, posix_sigev_oneshot_entry, ctx);
	if (!have_attr) {
		(void)pthread_attr_destroy(&attr);
	}
	if (ret != 0) {
		free(ctx);
		return -ret;
	}

	return 0;
}
#endif /* CONFIG_POSIX_THREADS */

int posix_sigev_notify_now(const struct sigevent *evp, k_tid_t default_target)
{
	switch (evp->sigev_notify) {
	case SIGEV_NONE:
		return 0;
#ifdef CONFIG_SIGNAL
	case SIGEV_SIGNAL:
		/* TODO(k_process): becomes process-directed with process support */
		return k_sig_queue((k_pid_t)default_target, z_sig_from_posix(evp->sigev_signo),
				   sigval_to_k(evp->sigev_value));
#ifdef SIGEV_THREAD_ID
	case SIGEV_THREAD_ID: {
		pthread_t th = (pthread_t)evp->sigev_notify_thread_id;

		return k_sig_queue((k_pid_t)to_k_thread(&th),
				   z_sig_from_posix(evp->sigev_signo),
				   sigval_to_k(evp->sigev_value));
	}
#endif /* SIGEV_THREAD_ID */
#endif /* CONFIG_SIGNAL */
#ifdef CONFIG_POSIX_THREADS
	case SIGEV_THREAD:
		return posix_sigev_oneshot_start(evp);
#endif /* CONFIG_POSIX_THREADS */
	default:
		return -ENOTSUP;
	}
}

#ifdef CONFIG_SYS_TIMER
int posix_sigev_to_notify(const struct sigevent *evp, const struct posix_sigev_thread *ctx,
			  struct k_timer_notify *out)
{
	switch (evp->sigev_notify) {
	case SIGEV_NONE:
		return 0;
#ifdef CONFIG_SIGNAL
	case SIGEV_SIGNAL:
		/* TODO(k_process): becomes process-directed with process support */
		out->target = (k_pid_t)k_current_get();
		out->signo = z_sig_from_posix(evp->sigev_signo);
		out->value = sigval_to_k(evp->sigev_value);
		return 1;
#ifdef SIGEV_THREAD_ID
	case SIGEV_THREAD_ID: {
		pthread_t th = (pthread_t)evp->sigev_notify_thread_id;
		struct k_thread *const target = to_k_thread(&th);

		if (target == NULL) {
			return -EINVAL;
		}
		out->target = (k_pid_t)target;
		out->signo = z_sig_from_posix(evp->sigev_signo);
		out->value = sigval_to_k(evp->sigev_value);
		return 1;
	}
#endif /* SIGEV_THREAD_ID */
#ifdef CONFIG_POSIX_THREADS
	case SIGEV_THREAD:
		/* the helper is woken with the internal signo; it calls the user function */
		out->target = (k_pid_t)to_k_thread(&ctx->tid);
		out->signo = POSIX_SIG_TIMER;
		out->value = sigval_to_k(evp->sigev_value);
		return 1;
#endif /* CONFIG_POSIX_THREADS */
#endif /* CONFIG_SIGNAL */
	default:
		return -ENOTSUP;
	}
}
#endif /* CONFIG_SYS_TIMER */
