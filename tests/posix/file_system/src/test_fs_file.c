/*
 * Copyright (c) 2018 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "test_fs.h"

const char test_str[] = "hello world!";
int file = -1;

static int test_file_open(void)
{
	int res;

	res = open(TEST_FILE, O_CREAT | O_RDWR, 0660);
	if (res < 0) {
		TC_ERROR("Failed opening file: %d, errno=%d\n", res, errno);
		/* FIXME: restructure tests as per #46897 */
		__ASSERT_NO_MSG(res >= 0);
	}

	file = res;

	return TC_PASS;
}

int test_file_write(void)
{
	ssize_t brw;
	off_t res;

	res = lseek(file, 0, SEEK_SET);
	if (res != 0) {
		TC_PRINT("lseek failed [%d]\n", (int)res);
		close(file);
		file = -1;
		return TC_FAIL;
	}

	brw = write(file, (char *)test_str, strlen(test_str));
	if (brw < 0) {
		TC_PRINT("Failed writing to file [%d]\n", (int)brw);
		close(file);
		file = -1;
		return TC_FAIL;
	}

	if (brw < strlen(test_str)) {
		TC_PRINT("Unable to complete write. Volume full.\n");
		TC_PRINT("Number of bytes written: [%d]\n", (int)brw);
		close(file);
		file = -1;
		return TC_FAIL;
	}

	return res;
}

static int test_file_read(void)
{
	ssize_t brw;
	off_t res;
	char read_buff[80];
	size_t sz = strlen(test_str);

	res = lseek(file, 0, SEEK_SET);
	if (res != 0) {
		TC_PRINT("lseek failed [%d]\n", (int)res);
		close(file);
		file = -1;
		return TC_FAIL;
	}

	brw = read(file, read_buff, sz);
	if (brw < 0) {
		TC_PRINT("Failed reading file [%d]\n", (int)brw);
		close(file);
		file = -1;
		return TC_FAIL;
	}

	read_buff[brw] = 0;

	if (strcmp(test_str, read_buff)) {
		TC_PRINT("Error - Data read does not match data written\n");
		TC_PRINT("Data read:\"%s\"\n\n", read_buff);
		return TC_FAIL;
	}

	/* Now test after non-zero lseek. */

	res = lseek(file, 2, SEEK_SET);
	if (res != 2) {
		TC_PRINT("lseek failed [%d]\n", (int)res);
		close(file);
		file = -1;
		return TC_FAIL;
	}

	brw = read(file, read_buff, sizeof(read_buff));
	if (brw < 0) {
		TC_PRINT("Failed reading file [%d]\n", (int)brw);
		close(file);
		file = -1;
		return TC_FAIL;
	}

	/* Check for array overrun */
	brw = (brw < 80) ? brw : brw - 1;

	read_buff[brw] = 0;

	if (strcmp(test_str + 2, read_buff)) {
		TC_PRINT("Error - Data read does not match data written\n");
		TC_PRINT("Data read:\"%s\"\n\n", read_buff);
		return TC_FAIL;
	}

	return TC_PASS;
}

static int test_file_close(void)
{
	int res = 0;

	if (file >= 0) {
		res = close(file);
		if (res < 0) {
			TC_ERROR("Failed closing file: %d, errno=%d\n", res, errno);
			/* FIXME: restructure tests as per #46897 */
			__ASSERT_NO_MSG(res == 0);
		}

		file = -1;
	}

	return res;
}


static int test_file_truncate(void)
{
	int res = 0;
	size_t truncate_size = sizeof(test_str) - 4;

	if (file < 0) {
		return res;
	}

	res = ftruncate(file, truncate_size);
	if (res) {
		TC_PRINT("Error truncating file [%d]\n", res);
		res = TC_FAIL;
	}

	close(file);
	file = -1;
	return res;
}

static int test_file_delete(void)
{
	int res;

	res = unlink(TEST_FILE);
	if (res) {
		TC_PRINT("Error deleting file [%d]\n", res);
		return res;
	}

	return res;
}

static void after_fn(void *unused)
{
	ARG_UNUSED(unused);

	test_file_close();
	unlink(TEST_FILE);
}

ZTEST_SUITE(posix_fs_file_test, NULL, test_mount, NULL, after_fn,
	    test_unmount);

/**
 * @brief Test for POSIX open API
 *
 * @details Test opens new file through POSIX open API.
 */
ZTEST(posix_fs_file_test, test_fs_open)
{
	/* FIXME: restructure tests as per #46897 */
	zassert_true(test_file_open() == TC_PASS);
}

/**
 * @brief Test for POSIX write API
 *
 * @details Test writes some data through POSIX write API.
 */
ZTEST(posix_fs_file_test, test_fs_write)
{
	/* FIXME: restructure tests as per #46897 */
	zassert_true(test_file_open() == TC_PASS);
	zassert_true(test_file_write() == TC_PASS);
}

/**
 * @brief Test for POSIX write API
 *
 * @details Test reads data back through POSIX read API.
 */
ZTEST(posix_fs_file_test, test_fs_read)
{
	/* FIXME: restructure tests as per #46897 */
	zassert_true(test_file_open() == TC_PASS);
	zassert_true(test_file_write() == TC_PASS);
	zassert_true(test_file_read() == TC_PASS);
}

/**
 * @brief Test for POSIX ftruncate API
 *
 * @details Test truncate the file through POSIX ftruncate API.
 */
ZTEST(posix_fs_file_test, test_fs_truncate)
{
	/* FIXME: restructure tests as per #46897 */
	zassert_true(test_file_open() == TC_PASS);
	zassert_true(test_file_write() == TC_PASS);
	zassert_true(test_file_truncate() == TC_PASS);
}

/**
 * @brief Test for POSIX close API
 *
 * @details Test closes the open file through POSIX close API.
 */
ZTEST(posix_fs_file_test, test_fs_close)
{
	/* FIXME: restructure tests as per #46897 */
	zassert_true(test_file_open() == TC_PASS);
	zassert_true(test_file_close() == TC_PASS);
}

/**
 * @brief Test for POSIX unlink API
 *
 * @details Test deletes a file through POSIX unlink API.
 */
ZTEST(posix_fs_file_test, test_fs_unlink)
{
	zassert_true(test_file_open() == TC_PASS);
	zassert_true(test_file_delete() == TC_PASS);
}

ZTEST(posix_fs_file_test, test_fs_fd_leak)
{
	const int reps =
	    MAX(CONFIG_POSIX_OPEN_MAX, ZVFS_OPEN_SIZE) + 5;

	for (int i = 0; i < reps; i++) {
		if (i > 0) {
			zassert_true(test_file_open() == TC_PASS);
		}

		if (i < reps - 1) {
			zassert_true(test_file_close() == TC_PASS);
		}
	}
}

/**
 * @brief fdopen()/fileno() round-trip (regression for zephyrproject-rtos/zephyr#108818)
 *
 * Previously zvfs_fdopen()/zvfs_fileno() cast a struct fd_entry * to FILE *,
 * which faulted as soon as the C library touched the FILE. The FILE object is
 * now backed by a statically-allocated pool (or an encoded fd for minimal libc),
 * so this must neither crash nor allocate, and fileno(fdopen(fd)) must equal fd.
 */
ZTEST(posix_fs_file_test, test_fs_fdopen_fileno)
{
	FILE *fp;
	int fd;

	zassert_true(test_file_open() == TC_PASS);
	fd = file;

	fp = fdopen(fd, "r+");
	zassert_not_null(fp, "fdopen() failed, errno=%d", errno);

	zassert_equal(fileno(fp), fd, "fileno(fdopen(%d)) != %d", fd, fd);

	zassert_ok(fclose(fp), "fclose() failed, errno=%d", errno);
	/* fclose() closed the underlying fd; prevent a double close in after_fn. */
	file = -1;
}

/**
 * @brief FILE I/O through a pooled fdopen()'d stream
 *
 * Exercises the path that crashed in #108818: writing and reading back through
 * the C library FILE API on top of a ZVFS file descriptor. Minimal libc provides
 * fopen()/fclose() via common libc and fdopen()/fileno() via POSIX device I/O,
 * plus fread() and fseek()/rewind() on ZVFS-backed streams.
 */
ZTEST(posix_fs_file_test, test_fs_fdopen_io)
{
	FILE *fp;
	size_t n;
	char buf[sizeof(test_str)] = {0};

	zassert_true(test_file_open() == TC_PASS);

	fp = fdopen(file, "r+");
	zassert_not_null(fp, "fdopen() failed, errno=%d", errno);

	n = fwrite(test_str, 1, strlen(test_str), fp);
	zassert_equal(n, strlen(test_str), "fwrite() wrote %zu (errno=%d)", n, errno);
	zassert_ok(fflush(fp), "fflush() failed, errno=%d", errno);

	zassert_equal(fseek(fp, 0, SEEK_SET), 0, "fseek() failed, errno=%d", errno);

	n = fread(buf, 1, strlen(test_str), fp);
	zassert_equal(n, strlen(test_str), "fread() read %zu (errno=%d)", n, errno);
	zassert_mem_equal(buf, test_str, strlen(test_str), "read back data mismatch");

	zassert_ok(fclose(fp), "fclose() failed, errno=%d", errno);
	file = -1;

	zassert_ok(test_file_delete());
}

/**
 * @brief fdopen()/fclose() cycles do not leak slots (no alloc)
 *
 * Loop more times than the pool can hold simultaneously. If fclose() did not
 * release the binding, fdopen() would eventually fail. With minimal libc the
 * FILE is an encoded fd rather than a pooled object, but the same invariant holds.
 */
ZTEST(posix_fs_file_test, test_fs_fdopen_no_leak)
{
	const int reps = MAX(CONFIG_POSIX_OPEN_MAX, ZVFS_OPEN_SIZE) + 5;

	for (int i = 0; i < reps; i++) {
		FILE *fp;

		zassert_true(test_file_open() == TC_PASS);

		fp = fdopen(file, "r+");
		zassert_not_null(fp, "fdopen() failed at iteration %d, errno=%d", i, errno);
		zassert_equal(fileno(fp), file, "fileno mismatch at iteration %d", i);

		zassert_ok(fclose(fp), "fclose() failed at iteration %d, errno=%d", i, errno);
		file = -1;
	}

	zassert_ok(test_file_delete());
}

ZTEST(posix_fs_file_test, test_file_open_truncate)
{
	struct stat buf = {0};

	zassert_ok(test_file_open());
	zassert_ok(test_file_write());
	zassert_ok(test_file_close());
	file = open(TEST_FILE, O_RDWR | O_TRUNC);
	zassert_not_equal(file, -1,
			  "File open failed for truncate mode");

	zassert_ok(test_file_close());
	zassert_ok(stat(TEST_FILE, &buf));
	zassert_equal(buf.st_size, 0, "Error: file is not truncated");
	zassert_ok(test_file_delete());
}
