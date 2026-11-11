/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_ADVISORY_INFO_TESTS_H_
#define POSIX_TESTS_ADVISORY_INFO_TESTS_H_

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

/*
 * The file system is mounted at "/", and /tmp exists in both worlds (the FAT
 * RAM disk under Zephyr, the host filesystem under CONFIG_NATIVE_LIBC), so
 * every path constant is identical for the linux_compat variant.
 */
#define FS_TMPDIR    "/tmp"
#define TEST_ROOT    FS_TMPDIR "/padv"
#define TEST_FILE    TEST_ROOT "/file.txt"
#define TEST_CONTENT "The quick brown fox jumps over the lazy dog!"

/* fixture descriptors, opened and granted per test */
extern int fs_test_fd;   /* TEST_FILE, O_RDWR */
extern int fs_test_rofd; /* TEST_FILE, O_RDONLY */
extern int fs_test_dirfd; /* TEST_ROOT, O_RDONLY */

#endif /* POSIX_TESTS_ADVISORY_INFO_TESTS_H_ */
