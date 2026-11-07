/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_SHARED_POSIX_IDENTITY_H_
#define ZEPHYR_LIB_POSIX_SHARED_POSIX_IDENTITY_H_

#include <sys/types.h>

/*
 * The identity every process runs with. There is one, the privileged one, so
 * the option groups that consult it (file access checks here, the user and
 * group interfaces of POSIX_USER_GROUPS) agree without calling each other.
 */
static inline uid_t posix_euid(void)
{
	return 0;
}

static inline gid_t posix_egid(void)
{
	return 0;
}

#endif /* ZEPHYR_LIB_POSIX_SHARED_POSIX_IDENTITY_H_ */
