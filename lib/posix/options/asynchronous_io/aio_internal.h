/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_OPTIONS_ASYNCHRONOUS_IO_AIO_INTERNAL_H_
#define ZEPHYR_LIB_POSIX_OPTIONS_ASYNCHRONOUS_IO_AIO_INTERNAL_H_

/*
 * This layer holds no per-request state of its own: struct aiocb carries the
 * sys_aio request handle, and every aio_*() call translates POSIX spellings
 * into one sys_aio_*() system call.
 */

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stddef.h>

#include <zephyr/posix/aio.h>
#include <zephyr/sys/aio.h>

#include "posix_internal.h"

/*
 * Control-block handle marking an entry lio_listio() could not submit: as on
 * Linux, such an entry completes with EINVAL retrievable through aio_error()
 * and aio_return() rather than rejecting the whole submission.
 */
#define Z_POSIX_AIO_REQ_FAILED ((struct sys_aio *)UINTPTR_MAX)

/*
 * Translate a sigevent into a sys_aio completion notification. Returns 0, or
 * -1 with errno set for an invalid notification.
 */
static inline int z_posix_aio_notify_from_sigevent(struct sys_aio_notify *notify,
					     const struct sigevent *sev)
{
	switch (sev->sigev_notify) {
	case SIGEV_NONE:
		break;
#ifdef CONFIG_SIGNAL
	case SIGEV_SIGNAL:
		if (sev->sigev_signo == 0) {
			/* nothing to generate */
			break;
		}
		notify->target = (k_pid_t)k_current_get();
		notify->signo = z_sig_from_posix(sev->sigev_signo);
		if (notify->signo < 0) {
			errno = EINVAL;
			return -1;
		}
		notify->value.sival_ptr = sev->sigev_value.sival_ptr;
		break;
#ifdef SIGEV_THREAD_ID
	case SIGEV_THREAD_ID: {
		pthread_t th = (pthread_t)sev->sigev_notify_thread_id;
		struct k_thread *const target = to_k_thread(&th);

		if (target == NULL) {
			errno = EINVAL;
			return -1;
		}
		notify->target = (k_pid_t)target;
		notify->signo = z_sig_from_posix(sev->sigev_signo);
		if (notify->signo < 0) {
			errno = EINVAL;
			return -1;
		}
		notify->value.sival_ptr = sev->sigev_value.sival_ptr;
		break;
	}
#endif /* SIGEV_THREAD_ID */
#endif /* CONFIG_SIGNAL */
	case SIGEV_THREAD: {
		if (sev->sigev_notify_function == NULL) {
			errno = EINVAL;
			return -1;
		}

		/*
		 * The service queue runs the notification function in a fresh
		 * (detached) thread per completion; both unions overlay an int
		 * and a pointer at offset zero, so the conversion is
		 * ABI-transparent.
		 */
		notify->fn = (void (*)(union k_sig_val))sev->sigev_notify_function;
		notify->value.sival_ptr = sev->sigev_value.sival_ptr;
		notify->fn_stack_size = 0;
		notify->fn_priority = k_thread_priority_get(k_current_get());

#ifdef _POSIX_THREADS
		const pthread_attr_t *attr = (const pthread_attr_t *)sev->sigev_notify_attributes;

		if (attr != NULL) {
			/*
			 * Translated at submission time: stack size and priority
			 * are honored (where those option groups are configured);
			 * detach state is always detached, other attributes do
			 * not apply.
			 */
#ifdef CONFIG_POSIX_THREAD_ATTR_STACKSIZE
			size_t stacksize = 0;

			if (pthread_attr_getstacksize(attr, &stacksize) == 0) {
				notify->fn_stack_size = stacksize;
			}
#endif /* CONFIG_POSIX_THREAD_ATTR_STACKSIZE */
#ifdef CONFIG_POSIX_THREAD_PRIORITY_SCHEDULING
			int policy = SCHED_RR;
			struct sched_param param = {0};

			if ((pthread_attr_getschedpolicy(attr, &policy) == 0) &&
			    (pthread_attr_getschedparam(attr, &param) == 0)) {
				notify->fn_priority =
					posix_to_zephyr_priority(param.sched_priority, policy);
			}
#endif /* CONFIG_POSIX_THREAD_PRIORITY_SCHEDULING */
		}
#endif /* _POSIX_THREADS */
		break;
	}
	default:
		errno = EINVAL;
		return -1;
	}

	return 0;
}

/* Submit one control block's operation, joining @a grp when non-NULL. */
static inline int z_posix_aio_submit(struct aiocb *acb, int op, struct sys_aio_group *grp)
{
	int ret;
	struct sys_aio *req = NULL;
	struct sys_aio_notify notify = {0};
	const struct sys_aio_notify *np = NULL;
	struct sys_aio_req sreq = {
		.op = op,
		.fd = acb->aio_fildes,
		.buf = (void *)acb->aio_buf,
		.len = acb->aio_nbytes,
	};

	if ((acb->aio_reqprio < 0) || (acb->aio_reqprio > AIO_PRIO_DELTA_MAX)) {
		errno = EINVAL;
		return -1;
	}
	/* Zephyr priorities are inverted: adding the delta lowers the request */
	sreq.prio = k_thread_priority_get(k_current_get()) + acb->aio_reqprio;

	if (op != SYS_AIO_OP_FSYNC) {
		if (acb->aio_offset < 0) {
			errno = EINVAL;
			return -1;
		}
		/* ignored below sys_aio for descriptors incapable of seeking */
		sreq.offset = (size_t)acb->aio_offset;
		sreq.flags = SYS_AIO_REQ_OFFSET;
	}

#ifdef _POSIX_REALTIME_SIGNALS
	/* a zero-initialized control block requests no notification */
	if ((acb->aio_sigevent.sigev_notify != 0) &&
	    (acb->aio_sigevent.sigev_notify != SIGEV_NONE)) {
		if (z_posix_aio_notify_from_sigevent(&notify, &acb->aio_sigevent) < 0) {
			return -1;
		}
		np = &notify;
	}
#endif /* _POSIX_REALTIME_SIGNALS */

	ret = sys_aio_submit(&sreq, np, grp, &req);
	if (ret < 0) {
		errno = (ret == -EAGAIN) ? EAGAIN : -ret;
		return -1;
	}

	acb->z_posix_aio_req = req;
	return 0;
}

#endif /* ZEPHYR_LIB_POSIX_OPTIONS_ASYNCHRONOUS_IO_AIO_INTERNAL_H_ */
