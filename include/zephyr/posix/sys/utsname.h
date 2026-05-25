/*
 * Copyright (c) 2023 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX system name identification (<sys/utsname.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_utsname.h.html">
 *      POSIX.1-2017 &lt;sys/utsname.h&gt;</a>
 *
 * @ingroup posix_option_group_single_process
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_UTSNAME_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_UTSNAME_H_

#include <zephyr/sys/util_macro.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @cond INTERNAL_HIDDEN */
#define _UTSNAME_NODENAME_LENGTH                                                                   \
	COND_CODE_1(CONFIG_POSIX_SINGLE_PROCESS, (CONFIG_POSIX_UNAME_VERSION_LEN), (0))
#define _UTSNAME_VERSION_LENGTH                                                                    \
	COND_CODE_1(CONFIG_POSIX_SINGLE_PROCESS, (CONFIG_POSIX_UNAME_VERSION_LEN), (0))
/** @endcond */

/** @brief System identification information returned by uname(). */
struct utsname {
	char sysname[sizeof("Zephyr")];              /**< Name of the operating system. */
	char nodename[_UTSNAME_NODENAME_LENGTH + 1]; /**< Network node hostname. */
	char release[sizeof("99.99.99-rc1")];        /**< Current release level. */
	char version[_UTSNAME_VERSION_LENGTH + 1];   /**< Current version level. */
	char machine[sizeof(CONFIG_ARCH)];            /**< Hardware type identifier. */
};

/**
 * @brief Get the name of the current system.
 * @ingroup posix_option_group_single_process
 * @param name Output: system identification structure to fill in.
 * @return 0 on success, or -1 with errno set on failure.
 */
int uname(struct utsname *name);


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_UTSNAME_H_ */
