/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_XSI_FILE_SYSTEM_TESTS_H_
#define POSIX_TESTS_XSI_FILE_SYSTEM_TESTS_H_

#include <zephyr/ztest.h>

/*
 * The XSI File System option group stands alone: the fixture is built with the Zephyr file
 * system API in kernel mode, and the tests observe through ZVFS system calls, which every
 * POSIX file function bottoms out in, so the suite builds with the other groups off.
 */
#define FS_TMPDIR     "/tmp"
#define TEST_ROOT     FS_TMPDIR "/pxsifs"
#define TEST_FILE     TEST_ROOT "/file.txt"
#define TEST_DIR      TEST_ROOT "/dir"
#define TEST_SUB      TEST_DIR "/sub.txt"
#define TEST_DEEP     TEST_DIR "/deep"
#define TEST_LEAF     TEST_DEEP "/leaf.txt"
#define TEST_EMPTY    TEST_ROOT "/empty"
#define TEST_LINK     TEST_ROOT "/link"
#define TEST_DANGLING TEST_ROOT "/dangling"
#define TEST_NOENT    TEST_ROOT "/nope"
#define TEST_SCRATCH  TEST_ROOT "/scratch"
#define TEST_CONTENT  "The quick brown fox jumps over the lazy dog!"

/* objects nftw() finds below TEST_ROOT, the root included */
#define TEST_TREE_OBJECTS 9

#endif /* POSIX_TESTS_XSI_FILE_SYSTEM_TESTS_H_ */
