/*
 * Copyright (c) 2018 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#if defined(CONFIG_NATIVE_LIBC)
#define TEST_MNTP	"/tmp/posix_fs_test"
#else
#define FATFS_MNTP	"/RAM:"
#define TEST_MNTP	FATFS_MNTP
#endif

#define TEST_ROOT	TEST_MNTP"/"
#define TEST_FILE	TEST_MNTP"/testfile.txt"
#define TEST_DIR	TEST_MNTP"/testdir"
#define TEST_DIR_FILE	TEST_MNTP"/testdir/testfile.txt"

extern const char test_str[];

void *test_mount(void);
void test_unmount(void *unused);
