/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/wait.h>
#include <unistd.h>

#include <zephyr/ztest.h>

#include "image_registry.h"
#include "spawn_internal.h"

static char *const spawn_argv[] = {"child", NULL};
static char *const spawn_envp[] = {NULL};

static void child_exit_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p3);

	zassert_equal((char *const *)p1, spawn_argv);
	zassert_equal((char *const *)p2, spawn_envp);
	_exit(7);
}

static void child_pgrp_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	_exit(getpgid(0) == getpid() ? 0 : 1);
}

static void child_sigmask_entry(void *p1, void *p2, void *p3)
{
	sigset_t cur;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	sigemptyset(&cur);
	(void)sigprocmask(SIG_SETMASK, NULL, &cur);
	_exit(((sigismember(&cur, SIGUSR2) == 1) && (sigismember(&cur, SIGUSR1) == 0)) ? 0 : 1);
}

IMAGE_REGISTRY_ENTRY_DEFINE(img_exit, "/bin/child", child_exit_entry);
IMAGE_REGISTRY_ENTRY_DEFINE(img_pgrp, "/bin/pgrp", child_pgrp_entry);
IMAGE_REGISTRY_ENTRY_DEFINE(img_sigmask, "/bin/sigmask", child_sigmask_entry);

ZTEST(posix_spawn, test_posix_spawn)
{
	pid_t pid = -1;
	int status = -1;

	/* unknown paths name nothing before exec exists */
	zassert_equal(posix_spawn(&pid, "/bin/nonesuch", NULL, NULL, spawn_argv, spawn_envp),
		      ENOENT);
	zassert_equal(posix_spawn(&pid, NULL, NULL, NULL, spawn_argv, spawn_envp), ENOENT);

	/* registry-resolved spawn with a pool-drawn leader; reap by pid */
	for (int i = 0; i < 3; i++) {
		pid = -1;
		zassert_ok(posix_spawn(&pid, "/bin/child", NULL, NULL, spawn_argv, spawn_envp));
		zassert_true(pid > 0);
		zassert_equal(waitpid(pid, &status, 0), pid);
		zassert_true(WIFEXITED(status));
		zassert_equal(WEXITSTATUS(status), 7);
	}

	/* file actions act on the child's descriptor table, never the caller's */
	posix_spawn_file_actions_t fa;
	eventfd_t val = 0;
	int efd = eventfd(0, 0);

	zassert_true(efd >= 0, "eventfd failed: %d", errno);
	zassert_ok(posix_spawn_file_actions_init(&fa));
	zassert_ok(posix_spawn_file_actions_addclose(&fa, efd));
	zassert_ok(posix_spawn(&pid, "/bin/child", &fa, NULL, spawn_argv, spawn_envp));
	zassert_ok(posix_spawn_file_actions_destroy(&fa));
	zassert_equal(waitpid(pid, &status, 0), pid);
	zassert_true(WIFEXITED(status) && (WEXITSTATUS(status) == 7));

	/* the addclose hit only the child's copy of the descriptor */
	zassert_ok(eventfd_write(efd, 9));
	zassert_ok(eventfd_read(efd, &val));
	zassert_equal(val, 9);
	zassert_ok(close(efd));
}

ZTEST(posix_spawn, test_posix_spawnp)
{
	pid_t pid = -1;
	int status = -1;

	/* no PATH search before exec: names resolve exactly as posix_spawn() */
	zassert_ok(posix_spawnp(&pid, "/bin/child", NULL, NULL, spawn_argv, spawn_envp));
	zassert_equal(waitpid(pid, &status, 0), pid);
	zassert_true(WIFEXITED(status) && (WEXITSTATUS(status) == 7));
}

ZTEST(posix_spawn, test_posix_spawnattr_init)
{
	posix_spawnattr_t attr;
	short flags = 0x7f;

	zassert_equal(posix_spawnattr_init(NULL), EINVAL);
	zassert_ok(posix_spawnattr_init(&attr));
	zassert_ok(posix_spawnattr_getflags(&attr, &flags));
	zassert_equal(flags, 0);
	zassert_ok(posix_spawnattr_destroy(&attr));
}

ZTEST(posix_spawn, test_posix_spawnattr_destroy)
{
	posix_spawnattr_t attr;

	zassert_equal(posix_spawnattr_destroy(NULL), EINVAL);
	zassert_ok(posix_spawnattr_init(&attr));
	zassert_ok(posix_spawnattr_destroy(&attr));
}

ZTEST(posix_spawn, test_posix_spawnattr_getflags)
{
	posix_spawnattr_t attr;
	short flags = -1;

	zassert_ok(posix_spawnattr_init(&attr));
	zassert_equal(posix_spawnattr_getflags(NULL, &flags), EINVAL);
	zassert_equal(posix_spawnattr_getflags(&attr, NULL), EINVAL);
	zassert_ok(posix_spawnattr_getflags(&attr, &flags));
	zassert_equal(flags, 0);
}

ZTEST(posix_spawn, test_posix_spawnattr_setflags)
{
	posix_spawnattr_t attr;
	short flags = 0;

	zassert_ok(posix_spawnattr_init(&attr));
	zassert_equal(posix_spawnattr_setflags(NULL, 0), EINVAL);
	zassert_ok(posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP));
	zassert_ok(posix_spawnattr_getflags(&attr, &flags));
	zassert_equal(flags, POSIX_SPAWN_SETPGROUP);
}

ZTEST(posix_spawn, test_posix_spawnattr_getpgroup)
{
	posix_spawnattr_t attr;
	pid_t pgroup = -1;

	zassert_ok(posix_spawnattr_init(&attr));
	zassert_equal(posix_spawnattr_getpgroup(NULL, &pgroup), EINVAL);
	zassert_equal(posix_spawnattr_getpgroup(&attr, NULL), EINVAL);
	zassert_ok(posix_spawnattr_getpgroup(&attr, &pgroup));
	zassert_equal(pgroup, 0);
}

ZTEST(posix_spawn, test_posix_spawnattr_setpgroup)
{
	posix_spawnattr_t attr;
	pid_t pgroup = -1;

	zassert_ok(posix_spawnattr_init(&attr));
	zassert_equal(posix_spawnattr_setpgroup(NULL, 0), EINVAL);
	zassert_ok(posix_spawnattr_setpgroup(&attr, 2));
	zassert_ok(posix_spawnattr_getpgroup(&attr, &pgroup));
	zassert_equal(pgroup, 2);

	/* end-to-end: SETPGROUP with pgroup 0 makes the child a group leader */
	posix_spawnattr_t sattr;
	pid_t pid = -1;
	int status = -1;

	zassert_ok(posix_spawnattr_init(&sattr));
	zassert_ok(posix_spawnattr_setflags(&sattr, POSIX_SPAWN_SETPGROUP));
	zassert_ok(posix_spawnattr_setpgroup(&sattr, 0));
	zassert_ok(posix_spawn(&pid, "/bin/pgrp", NULL, &sattr, spawn_argv, spawn_envp));
	zassert_equal(waitpid(pid, &status, 0), pid);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 0, "child was not its own group leader");
}

ZTEST(posix_spawn, test_posix_spawnattr_getschedparam)
{
	posix_spawnattr_t attr;
	struct sched_param param;

	zassert_ok(posix_spawnattr_init(&attr));
	zassert_equal(posix_spawnattr_getschedparam(NULL, &param), EINVAL);
	zassert_equal(posix_spawnattr_getschedparam(&attr, NULL), EINVAL);
	zassert_ok(posix_spawnattr_getschedparam(&attr, &param));
}

ZTEST(posix_spawn, test_posix_spawnattr_setschedparam)
{
	posix_spawnattr_t attr;
	struct sched_param param = {.sched_priority = 3};
	struct sched_param out = {0};

	zassert_ok(posix_spawnattr_init(&attr));
	zassert_equal(posix_spawnattr_setschedparam(NULL, &param), EINVAL);
	zassert_equal(posix_spawnattr_setschedparam(&attr, NULL), EINVAL);
	zassert_ok(posix_spawnattr_setschedparam(&attr, &param));
	zassert_ok(posix_spawnattr_getschedparam(&attr, &out));
	zassert_equal(out.sched_priority, 3);
}

ZTEST(posix_spawn, test_posix_spawnattr_getschedpolicy)
{
	posix_spawnattr_t attr;
	int policy = -1;

	zassert_ok(posix_spawnattr_init(&attr));
	zassert_equal(posix_spawnattr_getschedpolicy(NULL, &policy), EINVAL);
	zassert_equal(posix_spawnattr_getschedpolicy(&attr, NULL), EINVAL);
	zassert_ok(posix_spawnattr_getschedpolicy(&attr, &policy));
	zassert_equal(policy, 0);
}

ZTEST(posix_spawn, test_posix_spawnattr_setschedpolicy)
{
	posix_spawnattr_t attr;
	int policy = -1;

	zassert_ok(posix_spawnattr_init(&attr));
	zassert_equal(posix_spawnattr_setschedpolicy(NULL, 0), EINVAL);
	zassert_ok(posix_spawnattr_setschedpolicy(&attr, SCHED_RR));
	zassert_ok(posix_spawnattr_getschedpolicy(&attr, &policy));
	zassert_equal(policy, SCHED_RR);
}

ZTEST(posix_spawn, test_posix_spawnattr_getsigdefault)
{
	posix_spawnattr_t attr;
	sigset_t set;

	zassert_ok(posix_spawnattr_init(&attr));
	zassert_equal(posix_spawnattr_getsigdefault(NULL, &set), EINVAL);
	zassert_equal(posix_spawnattr_getsigdefault(&attr, NULL), EINVAL);
	zassert_ok(posix_spawnattr_getsigdefault(&attr, &set));
	zassert_false(sigismember(&set, SIGUSR1));
}

ZTEST(posix_spawn, test_posix_spawnattr_setsigdefault)
{
	posix_spawnattr_t attr;
	sigset_t set, out;

	zassert_ok(posix_spawnattr_init(&attr));
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	zassert_equal(posix_spawnattr_setsigdefault(NULL, &set), EINVAL);
	zassert_equal(posix_spawnattr_setsigdefault(&attr, NULL), EINVAL);
	zassert_ok(posix_spawnattr_setsigdefault(&attr, &set));
	zassert_ok(posix_spawnattr_getsigdefault(&attr, &out));
	zassert_true(sigismember(&out, SIGUSR1));
}

ZTEST(posix_spawn, test_posix_spawnattr_getsigmask)
{
	posix_spawnattr_t attr;
	sigset_t set;

	zassert_ok(posix_spawnattr_init(&attr));
	zassert_equal(posix_spawnattr_getsigmask(NULL, &set), EINVAL);
	zassert_equal(posix_spawnattr_getsigmask(&attr, NULL), EINVAL);
	zassert_ok(posix_spawnattr_getsigmask(&attr, &set));
	zassert_false(sigismember(&set, SIGUSR2));
}

ZTEST(posix_spawn, test_posix_spawnattr_setsigmask)
{
	posix_spawnattr_t attr;
	sigset_t set, out;

	zassert_ok(posix_spawnattr_init(&attr));
	sigemptyset(&set);
	sigaddset(&set, SIGUSR2);
	zassert_equal(posix_spawnattr_setsigmask(NULL, &set), EINVAL);
	zassert_equal(posix_spawnattr_setsigmask(&attr, NULL), EINVAL);
	zassert_ok(posix_spawnattr_setsigmask(&attr, &set));
	zassert_ok(posix_spawnattr_getsigmask(&attr, &out));
	zassert_true(sigismember(&out, SIGUSR2));

	/* end-to-end: SETSIGMASK seeds the child's initial signal mask */
	pid_t pid = -1;
	int status = -1;

	zassert_ok(posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK));
	zassert_ok(posix_spawn(&pid, "/bin/sigmask", NULL, &attr, spawn_argv, spawn_envp));
	zassert_equal(waitpid(pid, &status, 0), pid);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 0, "child mask did not match the spawn attr");
}

ZTEST(posix_spawn, test_posix_spawn_file_actions_init)
{
	posix_spawn_file_actions_t fa;

	zassert_equal(posix_spawn_file_actions_init(NULL), EINVAL);
	zassert_ok(posix_spawn_file_actions_init(&fa));
	zassert_equal(fa.num, 0);
	zassert_ok(posix_spawn_file_actions_destroy(&fa));
}

ZTEST(posix_spawn, test_posix_spawn_file_actions_destroy)
{
	posix_spawn_file_actions_t fa;

	zassert_equal(posix_spawn_file_actions_destroy(NULL), EINVAL);
	zassert_ok(posix_spawn_file_actions_init(&fa));
	zassert_ok(posix_spawn_file_actions_addclose(&fa, 3));
	zassert_ok(posix_spawn_file_actions_destroy(&fa));
	zassert_equal(fa.num, 0);
}

ZTEST(posix_spawn, test_posix_spawn_file_actions_addopen)
{
	posix_spawn_file_actions_t fa;

	zassert_ok(posix_spawn_file_actions_init(&fa));
	zassert_equal(posix_spawn_file_actions_addopen(NULL, 3, "/x", 0, 0), EINVAL);
	zassert_equal(posix_spawn_file_actions_addopen(&fa, 3, NULL, 0, 0), EINVAL);
	zassert_equal(posix_spawn_file_actions_addopen(&fa, -1, "/x", 0, 0), EBADF);
	zassert_ok(posix_spawn_file_actions_addopen(&fa, 3, "/x", O_RDONLY, 0));
	zassert_equal(fa.num, 1);
	zassert_equal(fa.actions[0].fildes, 3);
	zassert_ok(strcmp(fa.actions[0].path, "/x"));
	zassert_ok(posix_spawn_file_actions_destroy(&fa));
}

ZTEST(posix_spawn, test_posix_spawn_file_actions_addclose)
{
	posix_spawn_file_actions_t fa;

	zassert_ok(posix_spawn_file_actions_init(&fa));
	zassert_equal(posix_spawn_file_actions_addclose(NULL, 3), EINVAL);
	zassert_equal(posix_spawn_file_actions_addclose(&fa, -1), EBADF);
	/* growth across the initial capacity */
	for (int i = 0; i < 9; i++) {
		zassert_ok(posix_spawn_file_actions_addclose(&fa, 10 + i));
	}
	zassert_equal(fa.num, 9);
	zassert_equal(fa.actions[8].fildes, 18);
	zassert_ok(posix_spawn_file_actions_destroy(&fa));
}

ZTEST(posix_spawn, test_posix_spawn_file_actions_adddup2)
{
	posix_spawn_file_actions_t fa;

	zassert_ok(posix_spawn_file_actions_init(&fa));
	zassert_equal(posix_spawn_file_actions_adddup2(NULL, 3, 4), EINVAL);
	zassert_equal(posix_spawn_file_actions_adddup2(&fa, -1, 4), EBADF);
	zassert_equal(posix_spawn_file_actions_adddup2(&fa, 3, -1), EBADF);
	zassert_ok(posix_spawn_file_actions_adddup2(&fa, 3, 4));
	zassert_equal(fa.num, 1);
	zassert_equal(fa.actions[0].newfildes, 4);
	zassert_ok(posix_spawn_file_actions_destroy(&fa));
}

ZTEST_SUITE(posix_spawn, NULL, NULL, NULL, NULL, NULL);
