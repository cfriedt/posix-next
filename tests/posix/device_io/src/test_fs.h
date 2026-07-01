/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#if defined(CONFIG_NATIVE_LIBC)
#define TEST_FILE "/tmp/posix_device_io_testfile.txt"
#else
#define FATFS_MNTP "/RAM:"
#define TEST_FILE  FATFS_MNTP "/testfile.txt"
#endif

void *test_mount(void);
void test_unmount(void *unused);
