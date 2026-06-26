/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/ztest.h>

ZTEST(xsi_realtime_threads, test_pthread_mutexattr_getprotocol)
{
#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)
	int protocol = -1;
	pthread_mutexattr_t attr;

	zassert_equal(pthread_mutexattr_getprotocol(NULL, &protocol), EINVAL);
	zassert_equal(pthread_mutexattr_getprotocol(&attr, NULL), EINVAL);
	zassert_equal(pthread_mutexattr_getprotocol(NULL, NULL), EINVAL);

	zassert_ok(pthread_mutexattr_init(&attr));
	zassert_ok(pthread_mutexattr_getprotocol(&attr, &protocol));
	zassert_equal(protocol, PTHREAD_PRIO_NONE);
	zassert_ok(pthread_mutexattr_destroy(&attr));
#else
	ztest_test_skip();
#endif
}

ZTEST(xsi_realtime_threads, test_pthread_mutexattr_setprotocol)
{
#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)
	int protocol = -1;
	pthread_mutexattr_t attr;

	zassert_equal(pthread_mutexattr_setprotocol(NULL, PTHREAD_PRIO_NONE), EINVAL);

	zassert_ok(pthread_mutexattr_init(&attr));
	zassert_ok(pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_NONE));

	/* k_mutex implements priority inheritance, so PTHREAD_PRIO_INHERIT is accepted */
	zassert_ok(pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT));
	zassert_ok(pthread_mutexattr_getprotocol(&attr, &protocol));
	zassert_equal(protocol, PTHREAD_PRIO_INHERIT);

	/* priority ceilings are not implemented */
	zassert_equal(pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_PROTECT), ENOTSUP);
	zassert_equal(pthread_mutexattr_setprotocol(&attr, 42), EINVAL);
	zassert_ok(pthread_mutexattr_destroy(&attr));
#else
	ztest_test_skip();
#endif
}
