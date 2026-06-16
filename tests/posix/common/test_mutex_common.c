/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2024, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/ztest.h>

#define SLEEP_MS 100

static pthread_mutex_t mutex;

static void *normal_mutex_entry(void *p1)
{
	int i, rc;

	ARG_UNUSED(p1);

	for (i = 0; i < 3; i++) {
		rc = pthread_mutex_trylock(&mutex);
		if (rc == 0) {
			break;
		}
		k_msleep(SLEEP_MS);
	}

	zassert_false(rc, "try lock failed");
	TC_PRINT("mutex lock is taken\n");
	zassert_false(pthread_mutex_unlock(&mutex), "mutex unlock is failed");
	return NULL;
}

static void *recursive_mutex_entry(void *p1)
{
	ARG_UNUSED(p1);

	zassert_false(pthread_mutex_lock(&mutex), "mutex is not taken");
	zassert_false(pthread_mutex_lock(&mutex), "mutex is not taken 2nd time");
	TC_PRINT("recursive mutex lock is taken\n");
	zassert_false(pthread_mutex_unlock(&mutex), "mutex is not unlocked");
	zassert_false(pthread_mutex_unlock(&mutex), "mutex is not unlocked");
	return NULL;
}

void test_mutex_common(int type)
{
	__maybe_unused pthread_t th;
	__maybe_unused int protocol;
	int actual_type;
	pthread_mutexattr_t mut_attr;
	void *(*entry)(void *arg);

	switch (type) {
	case PTHREAD_MUTEX_NORMAL:
		entry = normal_mutex_entry;
		break;
	case PTHREAD_MUTEX_RECURSIVE:
		entry = recursive_mutex_entry;
		break;
	default:
		zassert_unreachable("unsupported mutex type for behavioral test");
		return;
	}

	zassert_ok(pthread_mutexattr_init(&mut_attr));
	zassert_ok(pthread_mutexattr_settype(&mut_attr, type), "setting mutex type is failed");
	zassert_ok(pthread_mutex_init(&mutex, &mut_attr), "mutex initialization is failed");

	zassert_ok(pthread_mutexattr_gettype(&mut_attr, &actual_type),
		   "reading mutex type is failed");
#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)
	zassert_not_ok(pthread_mutexattr_getprotocol(NULL, &protocol));
	zassert_not_ok(pthread_mutexattr_getprotocol(&mut_attr, NULL));
	zassert_not_ok(pthread_mutexattr_getprotocol(NULL, NULL));

	zassert_not_ok(pthread_mutexattr_setprotocol(&mut_attr, PTHREAD_PRIO_INHERIT));
	zassert_not_ok(pthread_mutexattr_setprotocol(&mut_attr, PTHREAD_PRIO_PROTECT));
	zassert_ok(pthread_mutexattr_setprotocol(&mut_attr, PTHREAD_PRIO_NONE));
	zassert_ok(pthread_mutexattr_getprotocol(&mut_attr, &protocol),
		   "reading mutex protocol is failed");
#endif /* defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT) */

	zassert_ok(pthread_mutexattr_destroy(&mut_attr));

	zassert_ok(pthread_mutex_lock(&mutex));

	zassert_equal(actual_type, type, "mutex type is not normal");
#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)
	zassert_equal(protocol, PTHREAD_PRIO_NONE, "mutex protocol is not prio_none");
#endif /* defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT) */

	if (CONFIG_SYS_THREAD_STACK_MAX > 0) {
		zassert_ok(pthread_create(&th, NULL, entry, NULL));
	}

	k_msleep(SLEEP_MS);
	zassert_ok(pthread_mutex_unlock(&mutex));

	if (CONFIG_SYS_THREAD_STACK_MAX > 0) {
		zassert_ok(pthread_join(th, NULL));
	}

	zassert_ok(pthread_mutex_destroy(&mutex), "Destroying mutex is failed");
}
