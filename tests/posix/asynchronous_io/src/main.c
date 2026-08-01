/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "asynchronous_io_tests.h"

#include <ff.h>
#include <zephyr/fs/fs.h>
#ifdef CONFIG_USERSPACE
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/internal/fdtable_priv.h>
#endif

#ifdef CONFIG_NATIVE_LIBC
#define AIO_TEST_FILE "/tmp/posix_aio_testfile.txt"
#else
/* an 8.3 name: FAT long file name support is not assumed */
#define AIO_TEST_FILE "/RAM:/aiotest.txt"
#endif

ZTEST_BMEM int aio_file_fd = -1;
ZTEST_BMEM int aio_filero_fd = -1;
ZTEST_BMEM int aio_efd = -1;

static FATFS fat_fs;
static struct fs_mount_t fs_mnt = {
	.type = FS_FATFS,
	.mnt_point = "/RAM:",
	.fs_data = &fat_fs,
};

#ifdef CONFIG_NATIVE_LIBC
/*
 * Under the host libc the notification signal is a real process-directed
 * signal: leaving it blocked and waiting for it with sigtimedwait() would
 * stall native_sim, and leaving it unhandled would kill the process. Capture
 * deliveries in a handler instead and consume them from a counter.
 *
 * aio_arm_sig() resets the counter so a completion that races ahead of
 * aio_accept_sig() (common when reap runs first) is still observed.
 */
static volatile int lc_count;
static siginfo_t lc_info;

static void lc_capture(int signo, siginfo_t *info, void *ctx)
{
	ARG_UNUSED(ctx);

	lc_info = *info;
	lc_info.si_signo = signo;
	lc_count++;
}

void aio_arm_sig(int signo)
{
	struct sigaction act = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = lc_capture,
	};

	lc_count = 0;
	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(signo, &act, NULL));
}

int aio_accept_sig(int signo, siginfo_t *info, int timeout_ms)
{
	ARG_UNUSED(signo);

	for (int elapsed = 0; elapsed <= timeout_ms; elapsed += 5) {
		if (lc_count > 0) {
			lc_count--;
			if (info != NULL) {
				*info = lc_info;
			}
			return lc_info.si_signo;
		}
		const struct timespec delay = {.tv_nsec = 5000000L};

		(void)nanosleep(&delay, NULL);
	}

	errno = EAGAIN;
	return -1;
}
#else
void aio_arm_sig(int signo)
{
	sigset_t set;

	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, signo));
	zassert_ok(sigprocmask(SIG_BLOCK, &set, NULL));
}

int aio_accept_sig(int signo, siginfo_t *info, int timeout_ms)
{
	sigset_t set;
	struct timespec timeout = {
		.tv_sec = timeout_ms / MSEC_PER_SEC,
		.tv_nsec = (timeout_ms % MSEC_PER_SEC) * NSEC_PER_MSEC,
	};

	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, signo));

	return sigtimedwait(&set, info, &timeout);
}
#endif /* CONFIG_NATIVE_LIBC */

static void grant_fd(int fd)
{
#ifdef CONFIG_USERSPACE
	k_object_access_all_grant(zvfs_fd_entry_get(fd));
#else
	ARG_UNUSED(fd);
#endif
}

static void *aio_test_setup(void)
{
	memset(&fat_fs, 0, sizeof(fat_fs));
	zassert_ok(fs_mount(&fs_mnt));

	return NULL;
}

static void aio_test_before(void *arg)
{
	ARG_UNUSED(arg);

	/* start from a fresh file each test */
#ifdef CONFIG_NATIVE_LIBC
	(void)unlink(AIO_TEST_FILE);
#else
	(void)fs_unlink(AIO_TEST_FILE);
#endif

	aio_file_fd = open(AIO_TEST_FILE, O_RDWR | O_CREAT, 0644);
	zassert_true(aio_file_fd >= 0, "open failed: %d", errno);
	zassert_equal(write(aio_file_fd, AIO_TEST_CONTENT, AIO_TEST_CONTENT_LEN),
		      AIO_TEST_CONTENT_LEN);

	aio_filero_fd = open(AIO_TEST_FILE, O_RDONLY);
	zassert_true(aio_filero_fd >= 0);

	aio_efd = eventfd(0, 0);
	zassert_true(aio_efd >= 0);

	grant_fd(aio_file_fd);
	grant_fd(aio_filero_fd);
	grant_fd(aio_efd);
}

static void aio_test_after(void *arg)
{
	ARG_UNUSED(arg);

	if (aio_efd >= 0) {
		(void)aio_cancel(aio_efd, NULL);
		(void)close(aio_efd);
		aio_efd = -1;
	}
	if (aio_filero_fd >= 0) {
		(void)close(aio_filero_fd);
		aio_filero_fd = -1;
	}
	if (aio_file_fd >= 0) {
		(void)close(aio_file_fd);
		aio_file_fd = -1;
	}
}

static void aio_test_teardown(void *arg)
{
	ARG_UNUSED(arg);

	(void)fs_unmount(&fs_mnt);
}

ZTEST_SUITE(posix_asynchronous_io, NULL, aio_test_setup, aio_test_before, aio_test_after,
	    aio_test_teardown);
