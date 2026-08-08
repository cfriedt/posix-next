/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_FILE_SYSTEM_TESTS_H_
#define POSIX_TESTS_FILE_SYSTEM_TESTS_H_

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

/*
 * The file system is mounted at "/", and /tmp exists in both worlds (the FAT
 * RAM disk under Zephyr, the host filesystem under CONFIG_NATIVE_LIBC), so
 * every path constant is identical for the linux_compat variant.
 */
#define FS_TMPDIR   "/tmp"
#define TEST_ROOT   FS_TMPDIR "/pfs"
#define TEST_FILE   TEST_ROOT "/file.txt"
#define TEST_EMPTY  TEST_ROOT "/empty.txt"
#define TEST_DIR    TEST_ROOT "/dir"
#define TEST_SUB    TEST_DIR "/sub.txt"
#define TEST_NOENT  TEST_ROOT "/nope.txt"
#define TEST_CONTENT "The quick brown fox jumps over the lazy dog!"

/* fixture descriptors, opened and granted per test */
extern int fs_test_fd;   /* TEST_FILE, O_RDWR */
extern int fs_test_rofd; /* TEST_FILE, O_RDONLY */

/* rebuild the baseline tree; kernel-mode machinery, used from before() */
void fs_test_reset(void);

#endif /* POSIX_TESTS_FILE_SYSTEM_TESTS_H_ */
