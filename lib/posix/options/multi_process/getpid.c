/*
 * Copyright (c) 2024, Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <unistd.h>

#include <zephyr/toolchain.h>

pid_t getpid(void)
{
	return z_posix_this_pid();
}
#ifdef CONFIG_POSIX_MULTI_PROCESS_ALIAS_GETPID
FUNC_ALIAS(getpid, _getpid, pid_t);
#endif /* CONFIG_POSIX_MULTI_PROCESS_ALIAS_GETPID */
