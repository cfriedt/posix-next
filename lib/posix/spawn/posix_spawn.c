/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "spawn_internal.h"
#include "posix_internal.h"

#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <signal.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>

/* a failed spawn leaves no child: abort the paused leader and reap the process */
static void spawn_child_discard(k_pid_t child)
{
	k_thread_abort(child);
	(void)k_waitpid(child, NULL, NULL, 0, K_FOREVER);
}

/* the child applies its file actions itself, on its own table, before its image */
static int spawn_file_actions_stage(const posix_spawn_file_actions_t *fa,
				    struct sys_clone_fd_action *acts, size_t max)
{
	if (fa->num > max) {
		return -E2BIG;
	}
	for (int i = 0; i < fa->num; i++) {
		const struct posix_spawn_file_action *act = &fa->actions[i];

		acts[i] = (struct sys_clone_fd_action){0};
		switch (act->type) {
		case POSIX_SPAWN_FILE_ACTION_OPEN:
			acts[i].op = SYS_CLONE_FD_OPEN;
			acts[i].fd = act->fildes;
			acts[i].oflag = act->oflag;
			acts[i].mode = act->mode;
			acts[i].path = act->path;
			break;
		case POSIX_SPAWN_FILE_ACTION_CLOSE:
			acts[i].op = SYS_CLONE_FD_CLOSE;
			acts[i].fd = act->fildes;
			break;
		case POSIX_SPAWN_FILE_ACTION_DUP2:
			acts[i].op = SYS_CLONE_FD_DUP2;
			acts[i].fd = act->fildes;
			acts[i].newfd = act->newfildes;
			break;
		}
	}

	return 0;
}

int posix_spawn(pid_t *pid, const char *path, const posix_spawn_file_actions_t *file_actions,
		const posix_spawnattr_t *attrp, char *const argv[], char *const envp[])
{
	int ret;
	k_pid_t child;
	sigset_t inherit;
	struct k_sig_set kmask;
	const sigset_t *maskp = &inherit;
	const struct posix_spawn_image *img;
	struct sys_clone_args args = {0};
	/* staged with the vectors: the kernel's budget bounds them */
	struct sys_clone_fd_action acts[(file_actions != NULL) ? MAX(file_actions->num, 1) : 1];

	if (path == NULL) {
		return ENOENT;
	}

	/* the vectors are copied into the child, which runs in user mode */
	args.argv = argv;
	args.envp = envp;
	if (IS_ENABLED(CONFIG_USERSPACE)) {
		args.options = K_USER;
	}
	img = posix_spawn_image_lookup(path);
	if ((img != NULL) && (img->entry != NULL)) {
		args.entry = img->entry;
	} else if (!IS_ENABLED(CONFIG_POSIX_EXEC_LLEXT)) {
		return ENOENT;
	}
	/* else: the image is loaded into the paused child by sys_exec_load() */
	if (file_actions != NULL) {
		if (spawn_file_actions_stage(file_actions, acts, file_actions->num) < 0) {
			return E2BIG;
		}
		args.fd_actions = acts;
		args.fd_actions_len = file_actions->num;
	}

	args.flags = SYS_CLONE_PAUSED;
	args.prio = k_thread_priority_get(k_current_get());

	if ((attrp != NULL) && ((attrp->flags & POSIX_SPAWN_SETSCHEDPARAM) != 0)) {
		int policy = ((attrp->flags & POSIX_SPAWN_SETSCHEDULER) != 0)
				     ? attrp->schedpolicy
				     : SCHED_RR;

		if (!is_posix_policy_prio_valid(attrp->schedparam.sched_priority, policy)) {
			return EINVAL;
		}
		args.prio = posix_to_zephyr_priority(attrp->schedparam.sched_priority, policy);
	}

	/* POSIX: the child gets the SETSIGMASK mask, else the caller's mask */
	if ((attrp != NULL) && ((attrp->flags & POSIX_SPAWN_SETSIGMASK) != 0)) {
		maskp = &attrp->sigmask;
	} else {
		(void)sigprocmask(SIG_SETMASK, NULL, &inherit);
	}
	args.sigmask = z_sig_set_from_posix(maskp, &kmask);

	ret = sys_clone(&args, &child);
	if (ret < 0) {
		if (ret == -E2BIG) {
			return E2BIG;
		}
		return (ret == -EINVAL) ? EINVAL : EAGAIN;
	}

	if (args.entry == NULL) {
		ret = sys_exec_load(child, path);
		if (ret < 0) {
			spawn_child_discard(child);
			switch (ret) {
			case -ENOEXEC:
				return ENOEXEC;
			case -ENOMEM:
				return ENOMEM;
			case -ENAMETOOLONG:
				return ENAMETOOLONG;
			default:
				return ENOENT;
			}
		}
	}

	/* the child is stopped: apply attributes, then start it */
	if (attrp != NULL) {
		if ((attrp->flags & POSIX_SPAWN_SETPGROUP) != 0) {
			/* pgroup 0 starts a new group led by the child (POSIX) */
			k_pgrp_t grp = (attrp->pgroup == 0) ? NULL
							    : sys_pgrp_find((int)attrp->pgroup);

			if (((attrp->pgroup != 0) && (grp == NULL)) ||
			    (sys_setpgid(child, grp) < 0)) {
				spawn_child_discard(child);
				return EINVAL;
			}
		}
	}

	if (pid != NULL) {
		*pid = (pid_t)sys_process_id(child);
	}

	k_thread_start(child);

	return 0;
}
