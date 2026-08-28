/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mqueue_internal.h"
#include "posix_internal.h"

#include <pthread.h>
#include <signal.h>

int mq_notify(mqd_t mqdes, const struct sigevent *notification)
{
	int ret;
	struct sys_msgq_notify notify = {0};

	if (notification == NULL) {
		ret = sys_msgq_notify((int)mqdes, NULL);
		if (ret == -EINVAL) {
			/* removing a registration that was never armed is not an error */
			ret = 0;
		}
		if (ret < 0) {
			errno = -ret;
			return -1;
		}
		return 0;
	}

	switch (notification->sigev_notify) {
	case SIGEV_NONE:
		/* consumed by the transition, generating nothing */
		break;
#ifdef CONFIG_SIGNAL
	case SIGEV_SIGNAL:
		notify.target = (k_pid_t)k_current_get();
		notify.signo = z_sig_from_posix(notification->sigev_signo);
		if (notify.signo < 0) {
			errno = EINVAL;
			return -1;
		}
		notify.value.sival_ptr = notification->sigev_value.sival_ptr;
		break;
#ifdef SIGEV_THREAD_ID
	case SIGEV_THREAD_ID: {
		pthread_t th = (pthread_t)notification->sigev_notify_thread_id;
		struct k_thread *const target = to_k_thread(&th);

		if (target == NULL) {
			errno = EINVAL;
			return -1;
		}
		notify.target = (k_pid_t)target;
		notify.signo = z_sig_from_posix(notification->sigev_signo);
		if (notify.signo < 0) {
			errno = EINVAL;
			return -1;
		}
		notify.value.sival_ptr = notification->sigev_value.sival_ptr;
		break;
	}
#endif /* SIGEV_THREAD_ID */
#endif /* CONFIG_SIGNAL */
	case SIGEV_THREAD: {
		if (notification->sigev_notify_function == NULL) {
			errno = EINVAL;
			return -1;
		}

		/*
		 * The kernel dispatcher runs the notification function in a fresh
		 * (detached) thread per arrival; both unions overlay an int and a
		 * pointer at offset zero, so the conversion is ABI-transparent.
		 */
		notify.fn = (void (*)(union k_sig_val))notification->sigev_notify_function;
		notify.value.sival_ptr = notification->sigev_value.sival_ptr;
		notify.fn_stack_size = 0;
		notify.fn_priority = k_thread_priority_get(k_current_get());

#ifdef _POSIX_THREADS
		const pthread_attr_t *attr =
			(const pthread_attr_t *)notification->sigev_notify_attributes;

		if (attr != NULL) {
			/*
			 * Translated at registration time: stack size and priority are
			 * honored (where those option groups are configured); detach
			 * state is always detached, other attributes do not apply.
			 */
#ifdef _POSIX_THREAD_ATTR_STACKSIZE
			size_t stacksize = 0;

			if (pthread_attr_getstacksize(attr, &stacksize) == 0) {
				notify.fn_stack_size = stacksize;
			}
#endif /* _POSIX_THREAD_ATTR_STACKSIZE */
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
			int policy = SCHED_RR;
			struct sched_param param = {0};

			if ((pthread_attr_getschedpolicy(attr, &policy) == 0) &&
			    (pthread_attr_getschedparam(attr, &param) == 0)) {
				notify.fn_priority =
					posix_to_zephyr_priority(param.sched_priority, policy);
			}
#endif /* _POSIX_THREAD_PRIORITY_SCHEDULING */
		}
#endif /* _POSIX_THREADS */
		break;
	}
	default:
		errno = EINVAL;
		return -1;
	}

	ret = sys_msgq_notify((int)mqdes, &notify);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}
