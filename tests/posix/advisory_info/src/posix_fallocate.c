/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "advisory_info_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_advisory_info, test_posix_fallocate)
{
	const size_t len = strlen(TEST_CONTENT);
	int ret;
	struct stat st;
	char buf[8];

	/* a region inside the file leaves it alone */
	zassert_ok(posix_fallocate(fs_test_fd, 0, len));
	zassert_ok(fstat(fs_test_fd, &st));
	zassert_equal(st.st_size, len);

	/* a region past the end extends it, with zeros */
	zassert_ok(posix_fallocate(fs_test_fd, len + 4, 8));
	zassert_ok(fstat(fs_test_fd, &st));
	zassert_equal(st.st_size, len + 12);
	zassert_equal(lseek(fs_test_fd, len, SEEK_SET), len);
	zassert_equal(read(fs_test_fd, buf, sizeof(buf)), sizeof(buf));
	zassert_mem_equal(buf, "\0\0\0\0\0\0\0\0", sizeof(buf));

	/* a shorter region never shrinks it */
	zassert_ok(posix_fallocate(fs_test_fd, 0, 1));
	zassert_ok(fstat(fs_test_fd, &st));
	zassert_equal(st.st_size, len + 12);

	if (IS_ENABLED(CONFIG_POSIX_SHARED_MEMORY_OBJECTS)) {
		/* a shared memory object is a file too; it is sized once */
		int shm = shm_open("/fallocate", O_CREAT | O_RDWR, 0600);

		zassert_true(shm >= 0, "shm_open() failed: %d", errno);
		zassert_ok(ftruncate(shm, 4096));
		ret = posix_fallocate(shm, 0, 4096);
		zassert_equal(ret, 0, "posix_fallocate() on a shared memory object: %d", ret);
		/* Zephyr sizes a shared memory object once; the host grows it */
		zassert_equal(posix_fallocate(shm, 0, 8192), IS_ENABLED(CONFIG_NATIVE_LIBC) ? 0 : EBUSY);
		zassert_ok(close(shm));
		zassert_ok(shm_unlink("/fallocate"));
	}

	zassert_equal(posix_fallocate(fs_test_fd, 0, 0), EINVAL);
	zassert_equal(posix_fallocate(fs_test_fd, -1, 8), EINVAL);
	zassert_equal(posix_fallocate(fs_test_fd, 0, -1), EINVAL);
	zassert_equal(posix_fallocate(-1, 0, 8), EBADF);
	/* not open for writing */
	zassert_equal(posix_fallocate(fs_test_rofd, 0, 8), EBADF);
}
