/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/thread.h>

#include "multi_process_internal.h"

struct exec_stage {
	struct z_posix_exec_run_args args;
	int argc;
	int envc;
	char *vec[];
};

static int exec_count_args(char *const list[], size_t *bytes)
{
	int n = 0;

	for (; (list != NULL) && (list[n] != NULL); n++) {
		*bytes += strlen(list[n]) + 1;
	}

	return n;
}

static char **exec_copy_args(char *const list[], int n, char **vec, char **buf)
{
	for (int i = 0; i < n; i++) {
		size_t len = strlen(list[i]) + 1;

		vec[i] = *buf;
		(void)memcpy(*buf, list[i], len);
		*buf += len;
	}
	vec[n] = NULL;

	return vec;
}

FUNC_NORETURN static void exec_run_image(const struct z_posix_exec_run_args *args, int argc,
					 char **argv, char **envp)
{
	int ret = 0;

	if (args->ext_main != NULL) {
		ret = args->ext_main(argc, argv, envp);
		/* a normal return unloads the image's extension */
		z_posix_exec_llext_reap(k_getpid());
	} else {
		args->entry((void *)argv, (void *)envp, NULL);
	}

	exit(ret);
}

static void exec_restart_entry(void *p1)
{
	struct exec_stage *st = p1;

	exec_run_image(&st->args, st->argc, &st->vec[0], &st->vec[st->argc + 1]);
}

FUNC_NORETURN static void exec_run_inline(const struct z_posix_exec_run_args *args)
{
	int argc = 0;

	while ((args->argv != NULL) && (args->argv[argc] != NULL)) {
		argc++;
	}

	exec_run_image(args, argc, (char **)args->argv, (char **)args->envp);
}

int z_posix_exec_args_check(char *const argv[], char *const envp[])
{
	size_t bytes = 0;

	if ((exec_count_args(argv, &bytes) > CONFIG_POSIX_EXEC_ARGS_MAX) ||
	    (exec_count_args(envp, &bytes) > CONFIG_POSIX_EXEC_ARGS_MAX) ||
	    (bytes > CONFIG_POSIX_EXEC_ARG_BYTES)) {
		errno = E2BIG;
		return -1;
	}

	return 0;
}

FUNC_NORETURN void z_posix_exec_run(const struct z_posix_exec_run_args *args)
{
	/* point of no return */
	z_posix_exec_prepare();

	{
		size_t strs = 0;
		int argc = exec_count_args(args->argv, &strs);
		int envc = exec_count_args(args->envp, &strs);
		size_t bytes = sizeof(struct exec_stage) +
			       ((size_t)(argc + envc + 2) * sizeof(char *)) + strs;
		struct exec_stage *st = sys_thread_stack_stage(bytes);

		if (st != NULL) {
			/*
			 * The vectors live in frames about to be abandoned:
			 * pack them above the restarted stack's base.
			 */
			char *cursor = (char *)&st->vec[argc + envc + 2];

			st->args = *args;
			st->argc = argc;
			st->envc = envc;
			(void)exec_copy_args(args->argv, argc, &st->vec[0], &cursor);
			(void)exec_copy_args(args->envp, envc, &st->vec[argc + 1], &cursor);
			(void)sys_thread_restart(exec_restart_entry, bytes);
		}
	}

	/* no stack-jump support or oversized vectors: run on the live frames */
	exec_run_inline(args);
}
