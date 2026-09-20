/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shell_internal.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <zephyr/sys/libc-hooks.h>
#include <zephyr/sys/sem.h>
#include <zephyr/sys/zvfs.h>
#include <zephyr/sys/zvfs_libc.h>
#include <zephyr/zvfs/pipe.h>

/* the process behind each open stream, for pclose() */
static Z_LIBC_DATA struct {
	FILE *stream;
	k_pid_t child;
} popen_streams[CONFIG_POSIX_SHELL_FUNC_POPEN_MAX];
static Z_LIBC_DATA SYS_SEM_DEFINE(popen_lock, 1, 1);

FILE *popen(const char *command, const char *type)
{
	int fds[2];
	int ret;
	int keep;
	int give;
	int child_fd;
	FILE *stream;
	k_pid_t child;
	struct sys_clone_fd_action acts[3];

	if (type == NULL || (type[0] != 'r' && type[0] != 'w') || type[1] != '\0') {
		errno = EINVAL;
		return NULL;
	}

	if (zvfs_pipe(fds, 0) < 0) {
		return NULL;
	}

	/* the child speaks through its own copy of one end, and closes both originals */
	if (type[0] == 'r') {
		keep = fds[0];
		give = fds[1];
		child_fd = STDOUT_FILENO;
	} else {
		keep = fds[1];
		give = fds[0];
		child_fd = STDIN_FILENO;
	}
	acts[0] = (struct sys_clone_fd_action){.op = SYS_CLONE_FD_DUP2, .fd = give,
					       .newfd = child_fd};
	acts[1] = (struct sys_clone_fd_action){.op = SYS_CLONE_FD_CLOSE, .fd = fds[0]};
	acts[2] = (struct sys_clone_fd_action){.op = SYS_CLONE_FD_CLOSE, .fd = fds[1]};

	ret = posix_shell_spawn(command, acts, ARRAY_SIZE(acts), &child);
	(void)zvfs_close(give);
	if (ret < 0) {
		(void)zvfs_close(keep);
		return NULL;
	}

	stream = zvfs_libc_fdopen(keep, type);
	if (stream == NULL) {
		(void)zvfs_close(keep);
		(void)posix_shell_wait(child);
		errno = EMFILE;
		return NULL;
	}

	(void)sys_sem_take(&popen_lock, K_FOREVER);
	ARRAY_FOR_EACH_PTR(popen_streams, ps) {
		if (ps->stream == NULL) {
			ps->stream = stream;
			ps->child = child;
			(void)sys_sem_give(&popen_lock);
			return stream;
		}
	}
	(void)sys_sem_give(&popen_lock);

	/* nowhere to remember the process: the stream cannot be closed properly */
	(void)fclose(stream);
	(void)posix_shell_wait(child);
	errno = EMFILE;
	return NULL;
}

int pclose(FILE *stream)
{
	k_pid_t child = NULL;

	(void)sys_sem_take(&popen_lock, K_FOREVER);
	ARRAY_FOR_EACH_PTR(popen_streams, ps) {
		if (ps->stream == stream) {
			child = ps->child;
			ps->stream = NULL;
			break;
		}
	}
	(void)sys_sem_give(&popen_lock);

	if (child == NULL) {
		errno = EBADF;
		return -1;
	}

	(void)fclose(stream);

	return posix_shell_wait(child);
}
