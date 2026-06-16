/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <pthread.h>
#include <sched.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/thread.h>

LOG_MODULE_DECLARE(pthread, CONFIG_PTHREAD_LOG_LEVEL);

int pthread_attr_getscope(const pthread_attr_t *_attr, int *contentionscope)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || contentionscope == NULL) {
		return EINVAL;
	}
	*contentionscope = attr->contentionscope;
	return 0;
}

int pthread_attr_setscope(pthread_attr_t *_attr, int contentionscope)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr)) {
		LOG_DBG("attr %p is not initialized", attr);
		return EINVAL;
	}
	if (!(contentionscope == PTHREAD_SCOPE_PROCESS ||
	      contentionscope == PTHREAD_SCOPE_SYSTEM)) {
		return EINVAL;
	}
	if (contentionscope == PTHREAD_SCOPE_PROCESS) {
		return ENOTSUP;
	}
	attr->contentionscope = contentionscope;
	return 0;
}

int pthread_attr_getinheritsched(const pthread_attr_t *_attr, int *inheritsched)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || inheritsched == NULL) {
		return EINVAL;
	}
	*inheritsched = attr->inheritsched;
	return 0;
}

int pthread_attr_setinheritsched(pthread_attr_t *_attr, int inheritsched)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr)) {
		return EINVAL;
	}

	if (inheritsched != PTHREAD_INHERIT_SCHED && inheritsched != PTHREAD_EXPLICIT_SCHED) {
		return EINVAL;
	}

	attr->inheritsched = inheritsched;
	return 0;
}

int pthread_setschedparam(pthread_t pthread, int policy, const struct sched_param *param)
{
	if (param == NULL || !valid_posix_policy(policy) ||
	    !is_posix_policy_prio_valid(param->sched_priority, policy)) {
		return EINVAL;
	}

	k_thread_priority_set(to_k_thread(&pthread),
			      posix_to_zephyr_priority(param->sched_priority, policy));

	return 0;
}

int pthread_setschedprio(pthread_t thread, int prio)
{
	int ret;
	int policy;
	struct sched_param param;

	ret = pthread_getschedparam(thread, &policy, &param);
	if (ret == 0) {
		param.sched_priority = prio;
		ret = pthread_setschedparam(thread, policy, &param);
	}

	return ret;
}

int pthread_getschedparam(pthread_t pthread, int *policy, struct sched_param *param)
{
	if (policy == NULL || param == NULL) {
		return EINVAL;
	}

	*param = (struct sched_param){
		.sched_priority = zephyr_to_posix_priority(
			k_thread_priority_get(to_k_thread(&pthread)), policy),
	};

	return 0;
}

int pthread_attr_getschedpolicy(const pthread_attr_t *_attr, int *policy)
{
	const struct posix_thread_attr *attr = (const struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || (policy == NULL)) {
		return EINVAL;
	}

	*policy = attr->schedpolicy;
	return 0;
}

int pthread_attr_setschedpolicy(pthread_attr_t *_attr, int policy)
{
	struct posix_thread_attr *attr = (struct posix_thread_attr *)_attr;

	if (!posix_thread_attr_is_valid(attr) || !valid_posix_policy(policy)) {
		return EINVAL;
	}

	attr->schedpolicy = policy;
	return 0;
}
