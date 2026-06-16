/*
 * Copyright (c) 2025 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/ztest.h>

ZTEST(xsi_realtime_threads, test_pthread_mutex_getprioceiling)
{
#if defined(_POSIX_THREAD_PRIO_PROTECT)
	zassert_equal(pthread_mutex_getprioceiling(NULL, NULL), ENOSYS);
#else
	ztest_test_skip();
#endif
}

ZTEST(xsi_realtime_threads, test_pthread_mutex_setprioceiling)
{
#if defined(_POSIX_THREAD_PRIO_PROTECT)
	zassert_equal(pthread_mutex_setprioceiling(NULL, 0, NULL), ENOSYS);
#else
	ztest_test_skip();
#endif
}

ZTEST(xsi_realtime_threads, test_pthread_mutexattr_getprioceiling)
{
#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)
	zassert_equal(pthread_mutexattr_getprioceiling(NULL, NULL), ENOSYS);
#else
	ztest_test_skip();
#endif
}

ZTEST(xsi_realtime_threads, test_pthread_mutexattr_setprioceiling)
{
#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)
	zassert_equal(pthread_mutexattr_setprioceiling(NULL, 0), ENOSYS);
#else
	ztest_test_skip();
#endif
}
