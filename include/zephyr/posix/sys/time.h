/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_TIME_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_TIME_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_TIMEVAL_DECLARED) && !defined(__timeval_defined)
struct timeval {
	time_t tv_sec;
	suseconds_t tv_usec;
};
#define _TIMEVAL_DECLARED
#define __timeval_defined
#endif

int gettimeofday(struct timeval *tv, void *tz);

#ifdef __cplusplus
}
#endif

#endif	/* ZEPHYR_INCLUDE_POSIX_SYS_TIME_H_ */
