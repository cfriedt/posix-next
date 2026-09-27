/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_MAPPED_FILES_TESTS_H_
#define POSIX_TESTS_MAPPED_FILES_TESTS_H_

#include <stddef.h>
#include <stdint.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

/*
 * The file system is mounted at "/", and /tmp exists in both worlds (the FAT
 * RAM disk under Zephyr, the host filesystem under CONFIG_NATIVE_LIBC).
 */
#define MF_TMPDIR "/tmp"
#define MF_FILE   MF_TMPDIR "/mapped.bin"

/* the fixture file holds this many full pages, plus half a page */
#define MF_FILE_PAGES 3

/* sysconf(_SC_PAGESIZE), captured by the fixture */
extern size_t mf_page;

static inline size_t mf_file_size(void)
{
	return MF_FILE_PAGES * mf_page + mf_page / 2;
}

/* the byte the fixture writes at file offset i */
static inline uint8_t mf_byte(size_t i)
{
	return (uint8_t)(i * 7 + (i >> 8));
}

/* the first mapping page boundary at or after the file's end */
static inline size_t mf_file_pages_rounded(void)
{
	return (MF_FILE_PAGES + 1) * mf_page;
}

#endif /* POSIX_TESTS_MAPPED_FILES_TESTS_H_ */
