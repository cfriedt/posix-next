/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_FILE_SYSTEM_EXT_TESTS_H_
#define POSIX_TESTS_FILE_SYSTEM_EXT_TESTS_H_

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

/*
 * The file system is mounted at "/", and /tmp exists in both worlds (the FAT
 * RAM disk under Zephyr, the host filesystem under CONFIG_NATIVE_LIBC), so
 * every path constant is identical for the linux_compat variant.
 */
#define FS_TMPDIR   "/tmp"
#define TEST_ROOT   FS_TMPDIR "/pfsext"
#define TEST_LINES  TEST_ROOT "/lines.txt"
#define TEST_FIELDS TEST_ROOT "/fields.txt"
#define TEST_EMPTY  TEST_ROOT "/empty.txt"
#define TEST_SCAN   TEST_ROOT "/scan"

/* three newline-terminated lines (the second exceeds any initial allocation)
 * and one unterminated tail
 */
#define TEST_LINE1 "alpha\n"
#define TEST_LINE2                                                                                 \
	"a second line that is long enough to force the line buffer through at least one growth\n"
#define TEST_LINE3 "charlie\n"
#define TEST_TAIL  "delta"

/* colon-delimited fields with an empty field and no trailing delimiter */
#define TEST_FIELD_DATA "one:two::three"

#endif /* POSIX_TESTS_FILE_SYSTEM_EXT_TESTS_H_ */
