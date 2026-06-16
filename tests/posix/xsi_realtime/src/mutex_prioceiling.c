/*
 * Copyright (c) 2025 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/ztest.h>

#if defined(_POSIX_THREAD_PRIO_PROTECT)
ZTEST(xsi_realtime, test_pthread_mutex_getprioceiling)
{
	zassert_equal(pthread_mutex_getprioceiling(NULL, NULL), ENOSYS);
}

ZTEST(xsi_realtime, test_pthread_mutex_setprioceiling)
{
	zassert_equal(pthread_mutex_setprioceiling(NULL, 0, NULL), ENOSYS);
}
#endif

#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)
ZTEST(xsi_realtime, test_pthread_mutexattr_getprioceiling)
{
	zassert_equal(pthread_mutexattr_getprioceiling(NULL, NULL), ENOSYS);
}

ZTEST(xsi_realtime, test_pthread_mutexattr_setprioceiling)
{
	zassert_equal(pthread_mutexattr_setprioceiling(NULL, 0), ENOSYS);
}
#endif
