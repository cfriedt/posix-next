/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <pthread.h>
#include <stdalign.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/thread.h>

FUNC_NORETURN
static void zephyr_thread_wrapper(void *arg1, void *arg2, void *arg3)
{
	void *(*fun_ptr)(void *arg) = arg2;

	k_thread_exit(fun_ptr(arg1));
	CODE_UNREACHABLE;
}

int pthread_create(pthread_t *ZRESTRICT thread, const pthread_attr_t *ZRESTRICT attr,
		   void *(*start_routine)(void *), void *ZRESTRICT arg)
{
	int ret;
	int prio;
	uint32_t options = 0;
	struct k_thread *k_thread;
	struct posix_thread_attr *attrp;
	struct posix_thread_attr default_attr __aligned(alignof(pthread_attr_t));

	if (attr == NULL) {
		attrp = &default_attr;
		posix_thread_attr_init(attrp);
		__ASSERT_NO_MSG(posix_thread_attr_is_valid(attrp));
	} else {
		attrp = (struct posix_thread_attr *)attr;
		if (!posix_thread_attr_is_valid(attrp)) {
			return EINVAL;
		}
	}

	if (attrp->detachstate == PTHREAD_CREATE_DETACHED) {
		options |= K_DETACHED;
	}

#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
	if (attrp->inheritsched == PTHREAD_INHERIT_SCHED) {
		prio = k_thread_priority_get(k_current_get());
	} else {
		prio = posix_to_zephyr_priority(attrp->priority, attrp->schedpolicy);
	}
#else
	prio = posix_to_zephyr_priority(attrp->priority, attrp->schedpolicy);
#endif /* _POSIX_THREAD_PRIORITY_SCHEDULING */

	ret = -sys_thread_create(&k_thread, attrp->stack, attrp->stacksize, attrp->guardsize,
				 zephyr_thread_wrapper, arg, start_routine, NULL, prio, options);

	if (ret == 0) {
		*thread = to_pthread_thread(k_thread);
	}

	return ret;
}
