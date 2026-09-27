/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#include <zephyr/kernel.h>

static union k_sig_val sigval_to_k(union sigval value)
{
	union k_sig_val kv;

	/* both unions overlay an int and a pointer at offset zero */
	kv.sival_ptr = value.sival_ptr;

	return kv;
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
		if (thread_via_signal) {
			/* dispatched by the kernel timer dispatcher (no pthreads needed) */
			if (!IS_ENABLED(CONFIG_TIMER_SIGNAL) || !IS_ENABLED(CONFIG_SYS_THREAD) ||
			    !IS_ENABLED(CONFIG_THREAD_DETACH)) {
				return -ENOTSUP;
			}
		} else if (!IS_ENABLED(CONFIG_POSIX_THREADS)) {
			/* one-shot notification threads (mq_notify) are pthreads */
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
int posix_sigev_to_notify(const struct sigevent *evp, struct k_timer_notify *out)
{
	*out = (struct k_timer_notify){0};

	switch (evp->sigev_notify) {
	case SIGEV_NONE:
		return 0;
#ifdef CONFIG_SIGNAL
	case SIGEV_SIGNAL:
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
#ifdef CONFIG_TIMER_SIGNAL
	case SIGEV_THREAD: {
		const pthread_attr_t *attr =
			(const pthread_attr_t *)evp->sigev_notify_attributes;

		/*
		 * The kernel timer dispatcher runs the notification function in a fresh
		 * (detached) thread per expiry; both unions overlay an int and a pointer
		 * at offset zero, so the function pointer conversion is ABI-transparent.
		 */
		out->fn = (void (*)(union k_sig_val))evp->sigev_notify_function;
		out->value = sigval_to_k(evp->sigev_value);
		out->fn_stack_size = 0;
		out->fn_priority = k_thread_priority_get(k_current_get());

		if (attr != NULL) {
			/*
			 * Translated at creation time: stack size and priority are
			 * honored (where those option groups are configured); detach
			 * state is always detached, other attributes (guard size,
			 * scope, inherit-sched) do not apply.
			 */
#ifdef _POSIX_THREAD_ATTR_STACKSIZE
			size_t stacksize = 0;

			if (pthread_attr_getstacksize(attr, &stacksize) == 0) {
				out->fn_stack_size = stacksize;
			}
#endif /* _POSIX_THREAD_ATTR_STACKSIZE */
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
			int policy = SCHED_RR;
			struct sched_param param = {0};

			if ((pthread_attr_getschedpolicy(attr, &policy) == 0) &&
			    (pthread_attr_getschedparam(attr, &param) == 0)) {
				out->fn_priority =
					posix_to_zephyr_priority(param.sched_priority, policy);
			}
#endif /* _POSIX_THREAD_PRIORITY_SCHEDULING */
		}
		return 1;
	}
#endif /* CONFIG_TIMER_SIGNAL */
#endif /* CONFIG_SIGNAL */
	default:
		return -ENOTSUP;
	}
}
#endif /* CONFIG_SYS_TIMER */
