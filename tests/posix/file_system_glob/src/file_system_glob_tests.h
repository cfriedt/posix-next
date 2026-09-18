/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_FILE_SYSTEM_GLOB_TESTS_H_
#define POSIX_TESTS_FILE_SYSTEM_GLOB_TESTS_H_

#include <glob.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

/*
 * The file system is mounted at "/", and /tmp exists in both worlds (the FAT
 * RAM disk under Zephyr, the host filesystem under CONFIG_NATIVE_LIBC), so
 * every path constant is identical for the linux_compat variant.
 *
 * TEST_ROOT holds a.txt, b.txt, c.dat, [ab].txt, .hidden, and the directory
 * sub with x.txt and y.txt.
 */
#define FS_TMPDIR    "/tmp"
#define TEST_ROOT    FS_TMPDIR "/pglob"
#define TEST_SUB     TEST_ROOT "/sub"
#define TEST_BRACKET TEST_ROOT "/[ab].txt"
#define TEST_HIDDEN  TEST_ROOT "/.hidden"
#define TEST_NODIR   TEST_ROOT "/nope"

#endif /* POSIX_TESTS_FILE_SYSTEM_GLOB_TESTS_H_ */
