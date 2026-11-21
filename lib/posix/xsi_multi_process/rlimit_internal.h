/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_XSI_MULTI_PROCESS_RLIMIT_INTERNAL_H_
#define ZEPHYR_LIB_POSIX_XSI_MULTI_PROCESS_RLIMIT_INTERNAL_H_

#include <sys/resource.h>

/* the build's limit on a resource, the same for every process; -EINVAL names no resource */
int z_posix_rlimit(int resource, rlim_t *limit);

#endif /* ZEPHYR_LIB_POSIX_XSI_MULTI_PROCESS_RLIMIT_INTERNAL_H_ */
