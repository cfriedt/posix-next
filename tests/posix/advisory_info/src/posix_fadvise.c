/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "advisory_info_tests.h"

#include <errno.h>
#include <fcntl.h>

ZTEST_USER(posix_advisory_info, test_posix_fadvise)
{
	static const int advice[] = {POSIX_FADV_NORMAL,   POSIX_FADV_RANDOM,
				     POSIX_FADV_SEQUENTIAL, POSIX_FADV_WILLNEED,
				     POSIX_FADV_DONTNEED,  POSIX_FADV_NOREUSE};

	/* every pattern, over a region, to the end, or the whole file, on any open mode */
	ARRAY_FOR_EACH(advice, i) {
		zassert_ok(posix_fadvise(fs_test_fd, 0, 16, advice[i]));
		zassert_ok(posix_fadvise(fs_test_fd, 8, 0, advice[i]));
		zassert_ok(posix_fadvise(fs_test_rofd, 0, 0, advice[i]));
	}

	/* a positive error number comes back, errno is not involved */
	zassert_equal(posix_fadvise(fs_test_fd, 0, 0, -1), EINVAL);
	zassert_equal(posix_fadvise(fs_test_fd, 0, 0, 4242), EINVAL);
	zassert_equal(posix_fadvise(-1, 0, 0, POSIX_FADV_NORMAL), EBADF);
	/* a negative region is rejected (the host takes it) */
	IF_NOT_NATIVE_LIBC({
		zassert_equal(posix_fadvise(fs_test_fd, -1, 0, POSIX_FADV_NORMAL), EINVAL);
		zassert_equal(posix_fadvise(fs_test_fd, 0, -1, POSIX_FADV_NORMAL), EINVAL);
	});
}
