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
 *      POSIX.1-2017 confstr()</a>
 *
 * @defgroup posix_confstr POSIX Configuration Strings
 * @ingroup posix_option_group_single_process
 * @{
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_CONFSTR_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_CONFSTR_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @brief confstr() name constants. */
enum {
	_CS_PATH,                            /**< Default value of PATH. */
	_CS_POSIX_V7_ILP32_OFF32_CFLAGS,    /**< Compilation flags for ILP32/OFF32 (POSIX.1-2008). */
	_CS_POSIX_V7_ILP32_OFF32_LDFLAGS,   /**< Linker flags for ILP32/OFF32 (POSIX.1-2008). */
	_CS_POSIX_V7_ILP32_OFF32_LIBS,      /**< Libraries for ILP32/OFF32 (POSIX.1-2008). */
	_CS_POSIX_V7_ILP32_OFFBIG_CFLAGS,   /**< Compilation flags for ILP32/OFFBIG (POSIX.1-2008). */
	_CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS,  /**< Linker flags for ILP32/OFFBIG (POSIX.1-2008). */
	_CS_POSIX_V7_ILP32_OFFBIG_LIBS,     /**< Libraries for ILP32/OFFBIG (POSIX.1-2008). */
	_CS_POSIX_V7_LP64_OFF64_CFLAGS,     /**< Compilation flags for LP64/OFF64 (POSIX.1-2008). */
	_CS_POSIX_V7_LP64_OFF64_LDFLAGS,    /**< Linker flags for LP64/OFF64 (POSIX.1-2008). */
	_CS_POSIX_V7_LP64_OFF64_LIBS,       /**< Libraries for LP64/OFF64 (POSIX.1-2008). */
	_CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS,   /**< Compilation flags for LPBIG/OFFBIG (POSIX.1-2008). */
	_CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS,  /**< Linker flags for LPBIG/OFFBIG (POSIX.1-2008). */
	_CS_POSIX_V7_LPBIG_OFFBIG_LIBS,     /**< Libraries for LPBIG/OFFBIG (POSIX.1-2008). */
	_CS_POSIX_V7_THREADS_CFLAGS,        /**< Compilation flags for threads (POSIX.1-2008). */
	_CS_POSIX_V7_THREADS_LDFLAGS,       /**< Linker flags for threads (POSIX.1-2008). */
	_CS_POSIX_V7_WIDTH_RESTRICTED_ENVS, /**< Environments with width restrictions (POSIX.1-2008). */
	_CS_V7_ENV,                          /**< Environments with width restrictions (alias). */
	_CS_POSIX_V6_ILP32_OFF32_CFLAGS,    /**< Compilation flags for ILP32/OFF32 (POSIX.1-2001). */
	_CS_POSIX_V6_ILP32_OFF32_LDFLAGS,   /**< Linker flags for ILP32/OFF32 (POSIX.1-2001). */
	_CS_POSIX_V6_ILP32_OFF32_LIBS,      /**< Libraries for ILP32/OFF32 (POSIX.1-2001). */
	_CS_POSIX_V6_ILP32_OFFBIG_CFLAGS,   /**< Compilation flags for ILP32/OFFBIG (POSIX.1-2001). */
	_CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS,  /**< Linker flags for ILP32/OFFBIG (POSIX.1-2001). */
	_CS_POSIX_V6_ILP32_OFFBIG_LIBS,     /**< Libraries for ILP32/OFFBIG (POSIX.1-2001). */
	_CS_POSIX_V6_LP64_OFF64_CFLAGS,     /**< Compilation flags for LP64/OFF64 (POSIX.1-2001). */
	_CS_POSIX_V6_LP64_OFF64_LDFLAGS,    /**< Linker flags for LP64/OFF64 (POSIX.1-2001). */
	_CS_POSIX_V6_LP64_OFF64_LIBS,       /**< Libraries for LP64/OFF64 (POSIX.1-2001). */
	_CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS,   /**< Compilation flags for LPBIG/OFFBIG (POSIX.1-2001). */
	_CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS,  /**< Linker flags for LPBIG/OFFBIG (POSIX.1-2001). */
	_CS_POSIX_V6_LPBIG_OFFBIG_LIBS,     /**< Libraries for LPBIG/OFFBIG (POSIX.1-2001). */
	_CS_POSIX_V6_WIDTH_RESTRICTED_ENVS, /**< Environments with width restrictions (POSIX.1-2001). */
	_CS_V6_ENV,                          /**< Environments with width restrictions (alias). */
};

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_CONFSTR_H_ */
