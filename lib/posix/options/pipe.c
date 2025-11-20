/*
 * Copyright (c) 2025 Atym, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "pipe_priv.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/fdtable.h>

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

static struct posix_pipe_desc desc_array[CONFIG_POSIX_OPEN_MAX];
static struct fd_op_vtable pipe_read_end_fd_op_vtable;
static struct fd_op_vtable pipe_write_end_fd_op_vtable;

static struct posix_pipe_desc *posix_pipe_alloc_obj()
{
	int i;
	struct posix_pipe_desc *ptr = NULL;
	unsigned int key = irq_lock();

	for (i = 0; i < CONFIG_POSIX_OPEN_MAX; i++) {
		if (desc_array[i].used == false) {
			ptr = &desc_array[i];

			ptr->ring_buffer = k_malloc(CONFIG_POSIX_PIPE_BUF);
			if (!ptr->ring_buffer) {
				ptr = NULL;
				break;
			}

			ptr->pipe = k_malloc(sizeof(*ptr->pipe));
			if (!ptr->pipe) {
				k_free(ptr->ring_buffer);
				ptr = NULL;
				break;
			}

			ptr->flags = 0;
			ptr->one_end_closed = false;
			ptr->used = true;
			break;
		}
	}

	irq_unlock(key);

	return ptr;
}

static inline void posix_pipe_free_obj(struct posix_pipe_desc *ptr)
{
	k_free(ptr->pipe);
	k_free(ptr->ring_buffer);
	ptr->used = false;
}

int pipe2(int pipefd[2], int flags)
{
	int rc, read_end, write_end;
	struct posix_pipe_desc *ptr = NULL;

	read_end = zvfs_reserve_fd();
	if (read_end < 0) {
		goto out_err;
	}

	write_end = zvfs_reserve_fd();
	if (write_end < 0) {
		goto out_err1;
	}

	ptr = posix_pipe_alloc_obj();
	if (ptr == NULL) {
		rc = -EMFILE;
		goto out_err2;
	}

	ptr->flags = flags;
	ptr->one_end_closed = false;

	k_pipe_init(ptr->pipe, ptr->ring_buffer, sizeof(ptr->ring_buffer));

	zvfs_finalize_fd(read_end, ptr, &pipe_read_end_fd_op_vtable);
	zvfs_finalize_fd(write_end, ptr, &pipe_write_end_fd_op_vtable);

	pipefd[0] = read_end;
	pipefd[1] = write_end;

	return 0;

out_err2:
	if (ptr != NULL) {
		posix_pipe_free_obj(ptr);
	}

	zvfs_free_fd(write_end);

out_err1:
	zvfs_free_fd(read_end);

out_err:
	errno = -rc;
	return -1;
}

int pipe(int pipefd[2])
{
	return pipe2(pipefd, 0);
}

static int pipe_close_vmeth(void *obj)
{
	struct posix_pipe_desc *ptr = obj;

	if (!ptr->one_end_closed) {
		/* first close, we just close the pipe */
		ptr->one_end_closed = true;
		k_pipe_close(ptr->pipe);

	} else {
		/* second close, we free everything */
		posix_pipe_free_obj(ptr);
	}

	return 0;
}

static int pipe_ioctl_vmeth(void *obj, unsigned int request, va_list args)
{
	int rc = 0;
	// struct posix_pipe_desc *ptr = obj;

	switch (request) {
	default:
		errno = EOPNOTSUPP;
		return -1;
	}

	if (rc < 0) {
		errno = -rc;
		return -1;
	}

	return rc;
}

/**
 * @brief Write to a pipe.
 */
static ssize_t pipe_write_vmeth(void *obj, const void *buffer, size_t count)
{
	ssize_t rc;
	size_t bytes_written = 0;
	struct posix_pipe_desc *ptr = obj;
	struct k_mutex *read_lock = NULL;
	struct k_mutex *write_lock = NULL;
	struct k_condvar *read_cond = NULL;
	struct k_condvar *write_cond = NULL;
	int err;

	if (ptr->flags & O_NONBLOCK) {
		rc = k_pipe_write(ptr->pipe, buffer, count, K_NO_WAIT);
		if (rc < 0) {
			errno = -rc;
			return -1;
		}
		return rc;
	}

	err = (int)zvfs_get_obj_lock_and_cond(obj, &pipe_read_end_fd_op_vtable, &read_lock,
					      &read_cond);
	__ASSERT((bool)err, "zvfs_get_obj_lock_and_cond() for read end failed");
	__ASSERT_NO_MSG(read_lock != NULL);
	__ASSERT_NO_MSG(read_cond != NULL);

	err = (int)zvfs_get_obj_lock_and_cond(obj, &pipe_write_end_fd_op_vtable, &write_lock,
					      &write_cond);
	__ASSERT((bool)err, "zvfs_get_obj_lock_and_cond() for write end failed");
	__ASSERT_NO_MSG(write_lock != NULL);
	__ASSERT_NO_MSG(write_cond != NULL);

	while (bytes_written < count) {
		rc = k_pipe_write(ptr->pipe, (char *)buffer + bytes_written, count - bytes_written,
				  K_NO_WAIT);
		// TODO: take mutex?
		// err = k_mutex_lock(read_lock, K_FOREVER);
		// __ASSERT(err == 0, "k_mutex_lock() failed: %d", err);
		err = k_condvar_signal(read_cond);
		__ASSERT(err == 0, "k_condvar_signal() failed: %d", err);
		if (rc == -EAGAIN) {
			err = k_condvar_wait(write_cond, write_lock, K_FOREVER);
			__ASSERT(err == 0, "k_condvar_wait() failed: %d", err);
		} else if (rc < 0) {
			errno = -rc;
			return -1;
		} else {
			bytes_written += rc;
		}
	}

	return bytes_written;
}

/**
 * @brief Read from a pipe.
 */
static ssize_t fs_read_vmeth(void *obj, void *buffer, size_t count)
{
	int err;
	ssize_t rc;
	struct k_mutex *read_lock = NULL;
	struct k_condvar *read_cond = NULL;
	struct k_mutex *write_lock = NULL;
	struct k_condvar *write_cond = NULL;
	struct posix_pipe_desc *ptr = obj;

	if (ptr->flags & O_NONBLOCK) {
		rc = k_pipe_read(ptr->pipe, buffer, count, K_NO_WAIT);
		if (rc < 0) {
			errno = -rc;
			return -1;
		}
		return rc;
	}

	err = (int)zvfs_get_obj_lock_and_cond(obj, &pipe_read_end_fd_op_vtable, &read_lock,
					      &read_cond);
	__ASSERT((bool)err, "zvfs_get_obj_lock_and_cond() for read end failed");
	__ASSERT_NO_MSG(read_lock != NULL);
	__ASSERT_NO_MSG(read_cond != NULL);

	err = (int)zvfs_get_obj_lock_and_cond(obj, &pipe_write_end_fd_op_vtable, &write_lock,
					      &write_cond);
	__ASSERT((bool)err, "zvfs_get_obj_lock_and_cond() for write end failed");
	__ASSERT_NO_MSG(write_lock != NULL);
	__ASSERT_NO_MSG(write_cond != NULL);

	while (true) {
		rc = k_pipe_read(ptr->pipe, buffer, count, K_NO_WAIT);
		// TODO: take mutex?
		// err = k_mutex_lock(write_lock, K_FOREVER);
		// __ASSERT(err == 0, "k_mutex_lock() failed: %d", err);
		err = k_condvar_signal(write_cond);
		__ASSERT(err == 0, "k_condvar_signal() failed: %d", err);
		if (rc >= 0) {
			return rc;
		} else if (rc == -EAGAIN) {
			err = k_condvar_wait(read_cond, read_lock, K_FOREVER);
			__ASSERT(err == 0, "k_condvar_wait() failed: %d", err);
		} else {
			errno = -rc;
			return -1;
		}
	}
}

static struct fd_op_vtable pipe_read_end_fd_op_vtable = {
	.read = fs_read_vmeth,
	.close = pipe_close_vmeth,
	.ioctl = pipe_ioctl_vmeth,
};

static struct fd_op_vtable pipe_write_end_fd_op_vtable = {
	.write = pipe_write_vmeth,
	.close = pipe_close_vmeth,
	.ioctl = pipe_ioctl_vmeth,
};
