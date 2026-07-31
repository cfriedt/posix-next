/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/mqueue.h>
#include <zephyr/sys/timeutil.h>

/*
 * mqd_t is a zvfs file descriptor and the queue itself is a kernel object
 * from the sys_msgq pool: this layer only translates POSIX spellings into
 * sys_msgq_*() calls.
 */

static int to_oflags(int oflags)
{
	int zflags = 0;

	switch (oflags & O_ACCMODE) {
	case O_WRONLY:
		zflags = ZVFS_O_WRONLY;
		break;
	case O_RDWR:
		zflags = ZVFS_O_RDWR;
		break;
	default:
		zflags = ZVFS_O_RDONLY;
		break;
	}

	if ((oflags & O_CREAT) != 0) {
		zflags |= ZVFS_O_CREAT;
	}
	if ((oflags & O_EXCL) != 0) {
		zflags |= ZVFS_O_EXCL;
	}
	if ((oflags & O_NONBLOCK) != 0) {
		zflags |= ZVFS_O_NONBLOCK;
	}

	return zflags;
}

mqd_t mq_open(const char *name, int oflags, ...)
{
	int ret;
	va_list va;
	struct k_msgq_attrs kattrs;
	const struct mq_attr *attrs = NULL;
	mode_t mode = 0;

	if ((oflags & O_CREAT) != 0) {
		va_start(va, oflags);
		BUILD_ASSERT(sizeof(mode_t) <= sizeof(int));
		mode = va_arg(va, unsigned int);
		attrs = va_arg(va, const struct mq_attr *);
		va_end(va);

		if ((attrs == NULL) || (attrs->mq_msgsize <= 0) || (attrs->mq_maxmsg <= 0)) {
			errno = EINVAL;
			return (mqd_t)-1;
		}

		kattrs = (struct k_msgq_attrs){
			.msg_size = (size_t)attrs->mq_msgsize,
			.max_msgs = (uint32_t)attrs->mq_maxmsg,
		};
	}

	ret = sys_msgq_open(name, to_oflags(oflags), (uint32_t)mode, (attrs == NULL) ? NULL
										   : &kattrs);
	if (ret < 0) {
		errno = -ret;
		return (mqd_t)-1;
	}

	return (mqd_t)ret;
}

int mq_close(mqd_t mqdes)
{
	return zvfs_close((int)mqdes);
}

int mq_unlink(const char *name)
{
	int ret = sys_msgq_unlink(name);

	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}

/*
 * Translate a transfer failure to errno. The kernel distinguishes "did not
 * wait" (-ENOMSG) from "waited and the deadline passed" (-EAGAIN), while POSIX
 * distinguishes a non-blocking descriptor (EAGAIN) from an expired timeout
 * (ETIMEDOUT). The two disagree in one case: a timed call whose deadline has
 * already passed does not wait either, so the descriptor's flags decide.
 */
static int mq_transfer_errno(mqd_t mqdes, int ret, bool timed)
{
	int oflags = 0;

	if (!timed) {
		return (ret == -ENOMSG) ? EAGAIN : -ret;
	}

	if (ret == -EAGAIN) {
		return ETIMEDOUT;
	}
	if (ret != -ENOMSG) {
		return -ret;
	}

	if ((sys_msgq_getattr((int)mqdes, NULL, &oflags) == 0) &&
	    ((oflags & ZVFS_O_NONBLOCK) != 0)) {
		return EAGAIN;
	}

	return ETIMEDOUT;
}

static int mq_send_common(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio,
			  k_timeout_t timeout, bool timed)
{
	int ret;

	if (msg_prio >= CONFIG_POSIX_MQ_PRIO_MAX) {
		errno = EINVAL;
		return -1;
	}

	ret = sys_msgq_send((int)mqdes, msg_ptr, msg_len, msg_prio, timeout);
	if (ret < 0) {
		errno = mq_transfer_errno(mqdes, ret, timed);
		return -1;
	}

	return 0;
}

static ssize_t mq_receive_common(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio,
				 k_timeout_t timeout, bool timed)
{
	int ret;
	uint32_t prio;

	ret = sys_msgq_receive((int)mqdes, msg_ptr, msg_len, &prio, timeout);
	if (ret < 0) {
		errno = mq_transfer_errno(mqdes, ret, timed);
		return -1;
	}

	if (msg_prio != NULL) {
		*msg_prio = prio;
	}

	return ret;
}

int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio)
{
	return mq_send_common(mqdes, msg_ptr, msg_len, msg_prio, K_FOREVER, false);
}

int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio,
		 const struct timespec *abstime)
{
	if ((abstime == NULL) || !timespec_is_valid(abstime)) {
		errno = EINVAL;
		return -1;
	}

	return mq_send_common(mqdes, msg_ptr, msg_len, msg_prio,
			      timespec_abs_to_timeout(SYS_CLOCK_REALTIME, abstime), true);
}

ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio)
{
	return mq_receive_common(mqdes, msg_ptr, msg_len, msg_prio, K_FOREVER, false);
}

ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio,
			const struct timespec *abstime)
{
	if ((abstime == NULL) || !timespec_is_valid(abstime)) {
		errno = EINVAL;
		return -1;
	}

	return mq_receive_common(mqdes, msg_ptr, msg_len, msg_prio,
				 timespec_abs_to_timeout(SYS_CLOCK_REALTIME, abstime), true);
}

int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat)
{
	int ret;
	int oflags;
	struct k_msgq_attrs kattrs;

	if (mqstat == NULL) {
		errno = EINVAL;
		return -1;
	}

	ret = sys_msgq_getattr((int)mqdes, &kattrs, &oflags);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	*mqstat = (struct mq_attr){
		.mq_flags = ((oflags & ZVFS_O_NONBLOCK) != 0) ? O_NONBLOCK : 0,
		.mq_maxmsg = (long)kattrs.max_msgs,
		.mq_msgsize = (long)kattrs.msg_size,
		.mq_curmsgs = (long)kattrs.used_msgs,
	};

	return 0;
}

int mq_setattr(mqd_t mqdes, const struct mq_attr *mqstat, struct mq_attr *omqstat)
{
	int ret;

	if (mqstat == NULL) {
		errno = EINVAL;
		return -1;
	}

	if ((mqstat->mq_flags & ~(long)O_NONBLOCK) != 0) {
		errno = EINVAL;
		return -1;
	}

	if (omqstat != NULL) {
		ret = mq_getattr(mqdes, omqstat);
		if (ret < 0) {
			return -1;
		}
	}

	ret = sys_msgq_setflags((int)mqdes,
				((mqstat->mq_flags & O_NONBLOCK) != 0) ? ZVFS_O_NONBLOCK : 0);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}

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
		/* TODO(k_process): becomes process-directed with process support */
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
