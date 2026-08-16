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

static int spawn_file_actions_apply(const posix_spawn_file_actions_t *fa)
{
	/*
	 * Before per-process descriptor tables exist (exec/M3), the fd space
	 * is global: applying the child-scope actions here is equivalent to
	 * applying them in the child.
	 */
	for (int i = 0; i < fa->num; i++) {
		const struct posix_spawn_file_action *act = &fa->actions[i];
		int ret = 0;

		switch (act->type) {
		case POSIX_SPAWN_FILE_ACTION_OPEN: {
			int fd = open(act->path, act->oflag, act->mode);

			if (fd < 0) {
				return errno;
			}
			if (fd != act->fildes) {
				ret = dup2(fd, act->fildes);
				(void)close(fd);
			}
			break;
		}
		case POSIX_SPAWN_FILE_ACTION_CLOSE:
			ret = close(act->fildes);
			break;
		case POSIX_SPAWN_FILE_ACTION_DUP2:
			ret = dup2(act->fildes, act->newfildes);
			break;
		}
		if (ret < 0) {
			return errno;
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

	if (path == NULL) {
		return ENOENT;
	}

	img = posix_spawn_image_lookup(path);
	if ((img == NULL) || (img->entry == NULL)) {
		return ENOENT;
	}

	if (file_actions != NULL) {
		ret = spawn_file_actions_apply(file_actions);
		if (ret != 0) {
			return ret;
		}
	}

	args.flags = SYS_CLONE_PAUSED;
	args.entry = img->entry;
	args.p1 = (void *)argv;
	args.p2 = (void *)envp;
	args.stack_size = CONFIG_POSIX_SPAWN_STACK_SIZE;
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
		return (ret == -EINVAL) ? EINVAL : EAGAIN;
	}

	/* the child is stopped: apply the attributes, then start it */
	if (attrp != NULL) {
		if ((attrp->flags & POSIX_SPAWN_SETPGROUP) != 0) {
			/* pgroup 0 starts a new group led by the child (POSIX) */
			k_pgrp_t grp = (attrp->pgroup == 0) ? NULL
							    : sys_pgrp_find((int)attrp->pgroup);

			if (((attrp->pgroup != 0) && (grp == NULL)) ||
			    (sys_setpgid(child, grp) < 0)) {
				k_thread_abort(child);
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
