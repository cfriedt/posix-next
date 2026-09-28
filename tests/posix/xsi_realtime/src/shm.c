/*
 * Copyright (c) 2024, Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <spawn.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/fdtable.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "image_registry.h"

#define _page_size COND_CODE_1(CONFIG_MMU, (CONFIG_MMU_PAGE_SIZE), (CONFIG_POSIX_PAGE_SIZE))

#define SHM_SIZE 8

#define VALID_SHM_PATH     "/foo"
#define INVALID_SHM_PATH   "foo"
#define EMPTY_SHM_PATH     ""
#define TOO_SHORT_SHM_PATH "/"

#define INVALID_MODE 0
#define VALID_MODE   0666

#define INVALID_FLAGS 0
#define VALID_FLAGS   (O_RDWR | O_CREAT)
#define CREATE_FLAGS  VALID_FLAGS
#define OPEN_FLAGS    (VALID_FLAGS & ~O_CREAT)

/* account for stdin, stdout, stderr */
#define N (ZVFS_OPEN_SIZE - 3)

/* we need to have at least 2 shared memory objects */
BUILD_ASSERT(N >= 2, "ZVFS_OPEN_SIZE must be > 4");

ZTEST(xsi_realtime, test_shm_open)
{
	int ret;
	int fd[N];
	struct stat st;

	/* a leftover of an earlier run on the host would carry a size */
	(void)shm_unlink(VALID_SHM_PATH);

	{
		/* degenerate error cases; the host libc accepts a slashless name and mode 0 */
		IF_NOT_NATIVE_LIBC({
			zassert_not_ok(shm_open(NULL, INVALID_FLAGS, INVALID_MODE));
			zassert_not_ok(shm_open(NULL, INVALID_FLAGS, VALID_MODE));
			zassert_not_ok(shm_open(NULL, VALID_FLAGS, INVALID_MODE));
			zassert_not_ok(shm_open(NULL, VALID_FLAGS, VALID_MODE));
			zassert_not_ok(shm_open(INVALID_SHM_PATH, VALID_FLAGS, VALID_MODE));
			zassert_not_ok(shm_open(VALID_SHM_PATH, VALID_FLAGS, INVALID_MODE));
		})
		zassert_not_ok(shm_open(EMPTY_SHM_PATH, VALID_FLAGS, VALID_MODE));
		zassert_not_ok(shm_open(TOO_SHORT_SHM_PATH, VALID_FLAGS, VALID_MODE));
		zassert_not_ok(shm_open(VALID_SHM_PATH, INVALID_FLAGS, INVALID_MODE));
		zassert_not_ok(shm_open(VALID_SHM_PATH, INVALID_FLAGS, VALID_MODE));
	}

	/* open / close 1 file descriptor referring to VALID_SHM_PATH */
	fd[0] = shm_open(VALID_SHM_PATH, VALID_FLAGS, VALID_MODE);
	zassert_true(fd[0] >= 0, "shm_open(%s, %x, %04o) failed: %d", VALID_SHM_PATH, VALID_FLAGS,
		     VALID_MODE, errno);

	/* should have size 0 and be a shared memory object (the host cannot tell) */
	zassert_ok(fstat(fd[0], &st));
	zassert_equal(st.st_size, 0);
	IF_NOT_NATIVE_LIBC({ zassert_true(S_TYPEISSHM(&st)); })

	/* technically, the order of close / shm_unlink can be reversed too */
	zassert_ok(close(fd[0]));
	ret = shm_unlink(VALID_SHM_PATH);
	zassert_true(ret == 0 || (ret == -1 && errno == ENOENT),
		     "unexpected return / errno from shm_unlink: %d / %d", ret, errno);

	/* open / close N file descriptors referring to VALID_SHM_PATH */
	for (size_t i = 0; i < N; ++i) {
		fd[i] = shm_open(VALID_SHM_PATH, i == 0 ? CREATE_FLAGS : OPEN_FLAGS, VALID_MODE);
		zassert_true(fd[i] >= 0, "shm_open(%s, %x, %04o) failed: %d", VALID_SHM_PATH,
			     VALID_FLAGS, VALID_MODE, errno);
	}
	zassert_ok(shm_unlink(VALID_SHM_PATH));
	for (size_t i = N; i > 0; --i) {
		zassert_ok(close(fd[i - 1]));
	}
}

ZTEST(xsi_realtime, test_shm_unlink)
{
	int fd;

	{
		/* degenerate error cases */
		IF_NOT_NATIVE_LIBC({ zassert_not_ok(shm_unlink(NULL)); })
		zassert_not_ok(shm_unlink(INVALID_SHM_PATH));
		zassert_not_ok(shm_unlink(EMPTY_SHM_PATH));
		zassert_not_ok(shm_unlink(TOO_SHORT_SHM_PATH));
	}

	/* open / close 1 file descriptor referring to VALID_SHM_PATH */
	fd = shm_open(VALID_SHM_PATH, VALID_FLAGS, VALID_MODE);
	zassert_true(fd >= 0, "shm_open(%s, %x, %04o) failed: %d", VALID_SHM_PATH, VALID_FLAGS,
		     VALID_MODE, errno);
	/* technically, the order of close / shm_unlink can be reversed too */
	zassert_ok(close(fd));
	zassert_ok(shm_unlink(VALID_SHM_PATH));
	/* should not be able to re-open the same path without O_CREAT */
	zassert_not_ok(shm_open(VALID_SHM_PATH, OPEN_FLAGS, VALID_MODE));
}

ZTEST(xsi_realtime, test_shm_read_write)
{
	int fd[N];

	for (size_t i = 0; i < N; ++i) {
		char cbuf = 0xff;

		fd[i] = shm_open(VALID_SHM_PATH, i == 0 ? CREATE_FLAGS : OPEN_FLAGS, VALID_MODE);
		zassert_true(fd[i] >= 0, "shm_open(%s, %x, %04o) failed: %d", VALID_SHM_PATH,
			     VALID_FLAGS, VALID_MODE, errno);
		if (i == 0) {
			/* size 0 on create: nothing to read or write (the host grows the object) */
			IF_NOT_NATIVE_LIBC({
				zassert_equal(write(fd[0], "", 1), 0,
					      "write() should fail on newly create shm fd with size 0");
			})
			zassert_equal(read(fd[0], &cbuf, 1), 0,
				      "read() should fail on newly create shm fd with size 0");

			BUILD_ASSERT(SHM_SIZE >= 1);
			zassert_ok(ftruncate(fd[0], SHM_SIZE));

			zassert_equal(write(fd[0], "\x42", 1), 1, "write() failed on fd %d: %d\n",
				      fd[0], errno);

			continue;
		}

		zassert_equal(read(fd[i], &cbuf, 1), 1, "read() failed on fd %d: %d\n", fd[i],
			      errno);
		zassert_equal(cbuf, 0x42,
			      "Failed to read byte over fd %d: expected: 0x%02x actual: 0x%02x",
			      fd[i], 0x42, cbuf);
	}

	for (size_t i = N; i > 0; --i) {
		zassert_ok(close(fd[i - 1]));
	}

	zassert_ok(shm_unlink(VALID_SHM_PATH));
}

#if defined(_POSIX_THREAD_PROCESS_SHARED) && defined(CONFIG_POSIX_SPAWN)
#define IPC_SHM_PATH "/ipc"
#define IPC_ROUNDS   4

/* the page both processes map: the turn passes back and forth under the shared lock */
struct ipc_page {
	pthread_mutex_t lock;
	pthread_cond_t turn_changed;
	sem_t mapped;
	int turn; /* 0: the parent's, 1: the child's */
	int rounds;
};
/* without an MMU a page is CONFIG_POSIX_PAGE_SIZE bytes, smaller than the struct on 64-bit */
#define IPC_SHM_SIZE ROUND_UP(sizeof(struct ipc_page), _page_size)

static void ipc_child_entry(void *p1, void *p2, void *p3)
{
	int fd = shm_open(IPC_SHM_PATH, O_RDWR, VALID_MODE);
	struct ipc_page *pg;
	int rounds = 0;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (fd < 0) {
		_exit(2);
	}
	pg = mmap(NULL, IPC_SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pg == MAP_FAILED) {
		_exit(3);
	}
	if (sem_post(&pg->mapped) != 0) {
		_exit(4);
	}

	for (int i = 0; i < IPC_ROUNDS; ++i) {
		if (pthread_mutex_lock(&pg->lock) != 0) {
			_exit(5);
		}
		while (pg->turn != 1) {
			if (pthread_cond_wait(&pg->turn_changed, &pg->lock) != 0) {
				_exit(6);
			}
		}
		pg->rounds++;
		rounds++;
		pg->turn = 0;
		(void)pthread_cond_signal(&pg->turn_changed);
		(void)pthread_mutex_unlock(&pg->lock);
	}

	(void)munmap(pg, IPC_SHM_SIZE);
	(void)close(fd);
	_exit((rounds == IPC_ROUNDS) ? 0 : 7);
}

IMAGE_REGISTRY_ENTRY_DEFINE(img_ipc, "/bin/ipc", ipc_child_entry);

/* a deadline for the waits on the child, so a child that died fails the test instead */
static struct timespec ipc_deadline(void)
{
	struct timespec ts;

	zassert_ok(clock_gettime(CLOCK_REALTIME, &ts));
	ts.tv_sec += 10;

	return ts;
}

/* synchronization objects in a mapped shared memory object work across processes */
static void shm_mmap_process_shared(void)
{
	struct timespec deadline;
	int fd = shm_open(IPC_SHM_PATH, CREATE_FLAGS, VALID_MODE);
	struct ipc_page *pg;
	pthread_mutexattr_t ma;
	pthread_condattr_t ca = {0};
	pid_t pid = -1;
	int status = -1;
	char *const argv[] = {"ipc", NULL};
	char *const envp[] = {NULL};

	zassert_true(fd >= 0, "shm_open(%s) failed: %d", IPC_SHM_PATH, errno);
	zassert_ok(ftruncate(fd, IPC_SHM_SIZE));
	pg = mmap(NULL, IPC_SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	zassert_not_equal(pg, MAP_FAILED, "mmap() failed: %d", errno);

	zassert_ok(pthread_mutexattr_init(&ma));
	zassert_ok(pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED));
	zassert_ok(pthread_mutex_init(&pg->lock, &ma));
	zassert_ok(pthread_mutexattr_destroy(&ma));
	zassert_ok(pthread_condattr_init(&ca));
	zassert_ok(pthread_condattr_setpshared(&ca, PTHREAD_PROCESS_SHARED));
	zassert_ok(pthread_cond_init(&pg->turn_changed, &ca));
	zassert_ok(pthread_condattr_destroy(&ca));
	zassert_ok(sem_init(&pg->mapped, 1, 0));
	pg->turn = 0;
	pg->rounds = 0;

	zassert_ok(posix_spawn(&pid, "/bin/ipc", NULL, NULL, argv, envp));
	deadline = ipc_deadline();
	zassert_ok(sem_timedwait(&pg->mapped, &deadline), "the child never mapped the page: %d",
		   errno);

	for (int i = 0; i < IPC_ROUNDS; ++i) {
		zassert_ok(pthread_mutex_lock(&pg->lock));
		pg->turn = 1;
		zassert_ok(pthread_cond_signal(&pg->turn_changed));
		while (pg->turn != 0) {
			zassert_ok(pthread_cond_timedwait(&pg->turn_changed, &pg->lock, &deadline),
				   "round %d: the child never took its turn", i);
		}
		zassert_ok(pthread_mutex_unlock(&pg->lock));
	}

	zassert_equal(waitpid(pid, &status, 0), pid, "waitpid failed: %d", errno);
	zassert_true(WIFEXITED(status), "status 0x%x", status);
	zassert_equal(WEXITSTATUS(status), 0, "the child left with %d", WEXITSTATUS(status));
	zassert_equal(pg->rounds, IPC_ROUNDS);

	zassert_ok(pthread_cond_destroy(&pg->turn_changed));
	zassert_ok(pthread_mutex_destroy(&pg->lock));
	zassert_ok(sem_destroy(&pg->mapped));
	zassert_ok(munmap(pg, IPC_SHM_SIZE));
	zassert_ok(close(fd));
	zassert_ok(shm_unlink(IPC_SHM_PATH));
}
#endif /* _POSIX_THREAD_PROCESS_SHARED && CONFIG_POSIX_SPAWN */

ZTEST(xsi_realtime, test_shm_mmap)
{
	int fd[N];
	void *addr[N];

	for (size_t i = 0; i < N; ++i) {
		fd[i] = shm_open(VALID_SHM_PATH, i == 0 ? CREATE_FLAGS : OPEN_FLAGS, VALID_MODE);
		zassert_true(fd[i] >= 0, "shm_open(%s, %x, %04o) failed : %d", VALID_SHM_PATH,
			     VALID_FLAGS, VALID_MODE, errno);

		if (i == 0) {
			/* cannot map shm of size zero */
			zassert_not_ok(mmap(NULL, _page_size, PROT_READ | PROT_WRITE, MAP_SHARED,
					    fd[0], 0));

			zassert_ok(ftruncate(fd[0], _page_size));
		}

		addr[i] = mmap(NULL, _page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd[i], 0);
		zassert_not_equal(MAP_FAILED, addr[i], "mmap() failed: %d", errno);

		if ((i & 1) == 0) {
			memset(addr[0], i & 0xff, _page_size);
		} else {
			zassert_mem_equal(addr[i], addr[i - 1], _page_size);
		}
	}

	for (size_t i = N; i > 0; --i) {
		zassert_ok(close(fd[i - 1]));
	}

	for (size_t i = N; i > 0; --i) {
		zassert_ok(munmap(addr[i - 1], _page_size));
		/*
		 * Note: for some reason, in Zephyr, unmapping a physical page once, removes all
		 * virtual mappings. When that behaviour changes, remove the break below and adjust
		 * shm.c accordingly.
		 */
		break;
	}

	zassert_ok(shm_unlink(VALID_SHM_PATH));

#if defined(_POSIX_THREAD_PROCESS_SHARED) && defined(CONFIG_POSIX_SPAWN)
	shm_mmap_process_shared();
#endif
}
