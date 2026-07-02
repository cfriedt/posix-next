/*
 * Copyright (c) 2024, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX configuration string constants (<sys/confstr.h>)
 *
 * Defines the @c _CS_* name constants used as the first argument to
 * confstr() to query implementation-defined configuration strings.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/functions/confstr.html">
 *      POSIX.1-2008 confstr()</a>
 *
 * @ingroup posix_option_group_single_process
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_CONFSTR_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_CONFSTR_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _CS_PATH

/** @brief confstr() name constants (POSIX.1-2008). */
#define _CS_PATH                               0
#define _CS_POSIX_V7_ILP32_OFF32_CFLAGS        1
#define _CS_POSIX_V6_ILP32_OFF32_CFLAGS       _CS_POSIX_V7_ILP32_OFF32_CFLAGS
#define _CS_XBS5_ILP32_OFF32_CFLAGS           _CS_POSIX_V7_ILP32_OFF32_CFLAGS
#define _CS_POSIX_V7_ILP32_OFF32_LDFLAGS       2
#define _CS_POSIX_V6_ILP32_OFF32_LDFLAGS      _CS_POSIX_V7_ILP32_OFF32_LDFLAGS
#define _CS_XBS5_ILP32_OFF32_LDFLAGS          _CS_POSIX_V7_ILP32_OFF32_LDFLAGS
#define _CS_POSIX_V7_ILP32_OFF32_LIBS          3
#define _CS_POSIX_V6_ILP32_OFF32_LIBS         _CS_POSIX_V7_ILP32_OFF32_LIBS
#define _CS_XBS5_ILP32_OFF32_LIBS             _CS_POSIX_V7_ILP32_OFF32_LIBS
#define _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS       4
#define _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS      _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS
#define _CS_XBS5_ILP32_OFFBIG_CFLAGS          _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS
#define _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS      5
#define _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS     _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS
#define _CS_XBS5_ILP32_OFFBIG_LDFLAGS         _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS
#define _CS_POSIX_V7_ILP32_OFFBIG_LIBS         6
#define _CS_POSIX_V6_ILP32_OFFBIG_LIBS        _CS_POSIX_V7_ILP32_OFFBIG_LIBS
#define _CS_XBS5_ILP32_OFFBIG_LIBS            _CS_POSIX_V7_ILP32_OFFBIG_LIBS
#define _CS_POSIX_V7_LP64_OFF64_CFLAGS         7
#define _CS_POSIX_V6_LP64_OFF64_CFLAGS        _CS_POSIX_V7_LP64_OFF64_CFLAGS
#define _CS_XBS5_LP64_OFF64_CFLAGS            _CS_POSIX_V7_LP64_OFF64_CFLAGS
#define _CS_POSIX_V7_LP64_OFF64_LDFLAGS        8
#define _CS_POSIX_V6_LP64_OFF64_LDFLAGS       _CS_POSIX_V7_LP64_OFF64_LDFLAGS
#define _CS_XBS5_LP64_OFF64_LDFLAGS           _CS_POSIX_V7_LP64_OFF64_LDFLAGS
#define _CS_POSIX_V7_LP64_OFF64_LIBS           9
#define _CS_POSIX_V6_LP64_OFF64_LIBS          _CS_POSIX_V7_LP64_OFF64_LIBS
#define _CS_XBS5_LP64_OFF64_LIBS              _CS_POSIX_V7_LP64_OFF64_LIBS
#define _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS      10
#define _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS      _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS
#define _CS_XBS5_LPBIG_OFFBIG_CFLAGS          _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS
#define _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS     11
#define _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS     _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS
#define _CS_XBS5_LPBIG_OFFBIG_LDFLAGS         _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS
#define _CS_POSIX_V7_LPBIG_OFFBIG_LIBS        12
#define _CS_POSIX_V6_LPBIG_OFFBIG_LIBS        _CS_POSIX_V7_LPBIG_OFFBIG_LIBS
#define _CS_XBS5_LPBIG_OFFBIG_LIBS            _CS_POSIX_V7_LPBIG_OFFBIG_LIBS
#define _CS_POSIX_V7_WIDTH_RESTRICTED_ENVS    13
#define _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS    _CS_POSIX_V7_WIDTH_RESTRICTED_ENVS
#define _CS_XBS5_WIDTH_RESTRICTED_ENVS        _CS_POSIX_V7_WIDTH_RESTRICTED_ENVS
#define _CS_POSIX_V7_THREADS_CFLAGS           14
#define _CS_POSIX_V7_THREADS_LDFLAGS          15
#define _CS_V7_ENV                            16
#define _CS_V6_ENV                            _CS_V7_ENV
#define _CS_LFS_CFLAGS                        17
#define _CS_LFS_LDFLAGS                       18
#define _CS_LFS_LIBS                          19

#endif /* _CS_PATH */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_CONFSTR_H_ */
