/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_WIDE_CHAR_DEVICE_IO_TESTS_H_
#define POSIX_TESTS_WIDE_CHAR_DEVICE_IO_TESTS_H_

#include <stdio.h>
#include <wchar.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

/*
 * The file system is mounted at "/", and /tmp exists in both worlds (the FAT
 * RAM disk under Zephyr, the host filesystem under CONFIG_NATIVE_LIBC), so
 * every path constant is identical for the linux_compat variant.
 */
#define FS_TMPDIR    "/tmp"
#define TEST_ROOT    FS_TMPDIR "/pwcio"
#define TEST_IN      TEST_ROOT "/in.txt"
#define TEST_OUT     TEST_ROOT "/out.txt"
#define TEST_CONTENT L"hello 42\nsecond line\n"

/*
 * Every file is written and read with the wide functions of the C library
 * under test: whether a wide stream holds a multibyte encoding (glibc) or
 * raw wide characters (Picolibc), a round trip through the same library holds.
 * TEST_IN holds TEST_CONTENT; TEST_OUT is empty.
 */
FILE *test_open_in(void);
FILE *test_open_out(void);
/* the wide characters TEST_OUT holds */
const wchar_t *test_read_out(void);

/* the standard streams read TEST_IN and write TEST_OUT between these calls */
void test_swap_std_in(void);
void test_swap_std_out(void);
void test_restore_std(void);

#endif /* POSIX_TESTS_WIDE_CHAR_DEVICE_IO_TESTS_H_ */
