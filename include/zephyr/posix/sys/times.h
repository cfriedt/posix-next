/*
 * Copyright (c) 2025 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX process CPU time accounting (<sys/times.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_times.h.html">
 *      POSIX.1-2017 &lt;sys/times.h&gt;</a>
 *
 * @ingroup posix_option_group_multi_process
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_TIMES_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_TIMES_H_

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#if !defined(_TMS_DECLARED) && !defined(__tms_defined)
/** @brief CPU time accounting structure filled in by times(). */
struct tms {
	clock_t tms_utime;  /**< User CPU time of the process. */
	clock_t tms_stime;  /**< System CPU time of the process. */
	clock_t tms_cutime; /**< User CPU time of waited-for children. */
	clock_t tms_cstime; /**< System CPU time of waited-for children. */
};
#define _TMS_DECLARED
#define __tms_defined
#endif

/**
 * @brief Get the process and child process times.
 * @ingroup posix_option_group_multi_process
 * @param buf Output: filled with CPU times for the calling process.
 * @return Elapsed real time in clock ticks since an arbitrary epoch, or
 *         (clock_t)-1 with errno set on failure.
 */
clock_t times(struct tms *buf);

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_TIMES_H_ */
