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

#ifdef CONFIG_POSIX_EXEC_LLEXT
/* single consumer of the llext exec entry point (multi_process) */
int z_posix_exec_llext(const char *path, char *const argv[], char *const envp[]);

/*
 * A path that names no prelinked image is loaded from the file system by the
 * child itself. The path, arguments, and environment are snapshotted into a
 * claimed slot at spawn time - the caller's vectors need not outlive the
 * call - and the child's entry copies them onto its own stack, releases the
 * slot, and execs.
 */
static struct {
	atomic_t used;
	uint16_t argc;
	uint16_t envc;
	char path[CONFIG_POSIX_EXEC_PATH_MAX];
	char args[CONFIG_POSIX_EXEC_ARG_BYTES];
} spawn_exec_slots[CONFIG_POSIX_EXEC_LLEXT_MAX];

static void spawn_exec_trampoline(void *p1, void *p2, void *p3)
{
	size_t slot = (size_t)p3;
	char path[CONFIG_POSIX_EXEC_PATH_MAX];
	char args[CONFIG_POSIX_EXEC_ARG_BYTES];
	char *argv[CONFIG_POSIX_EXEC_ARGS_MAX + 1];
	char *envp[CONFIG_POSIX_EXEC_ARGS_MAX + 1];
	char *cursor = args;
	int argc = spawn_exec_slots[slot].argc;
	int envc = spawn_exec_slots[slot].envc;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);

	strcpy(path, spawn_exec_slots[slot].path);
	(void)memcpy(args, spawn_exec_slots[slot].args, sizeof(args));
	atomic_clear(&spawn_exec_slots[slot].used);

	for (int i = 0; i < argc; i++) {
		argv[i] = cursor;
		cursor += strlen(cursor) + 1;
	}
	argv[argc] = NULL;
	for (int i = 0; i < envc; i++) {
		envp[i] = cursor;
		cursor += strlen(cursor) + 1;
	}
	envp[envc] = NULL;

	(void)z_posix_exec_llext(path, argv, envp);
	_exit(127);
}

static int spawn_exec_pack(char *dst, size_t size, char *const list[], size_t *off, uint16_t *n)
{
	*n = 0;

	for (int i = 0; (list != NULL) && (list[i] != NULL); i++) {
		size_t len = strlen(list[i]) + 1;

		if (i >= CONFIG_POSIX_EXEC_ARGS_MAX) {
			return -E2BIG;
		}
		if ((*off + len) > size) {
			return -E2BIG;
		}
		(void)memcpy(&dst[*off], list[i], len);
		*off += len;
		*n = (uint16_t)(i + 1);
	}

	return 0;
}

static int spawn_exec_slot_claim(const char *path, char *const argv[], char *const envp[])
{
	if (strlen(path) >= CONFIG_POSIX_EXEC_PATH_MAX) {
		return -ENAMETOOLONG;
	}

	for (size_t i = 0; i < ARRAY_SIZE(spawn_exec_slots); i++) {
		if (atomic_cas(&spawn_exec_slots[i].used, 0, 1)) {
			size_t off = 0;

			strcpy(spawn_exec_slots[i].path, path);
			if ((spawn_exec_pack(spawn_exec_slots[i].args,
					     sizeof(spawn_exec_slots[i].args), argv, &off,
					     &spawn_exec_slots[i].argc) < 0) ||
			    (spawn_exec_pack(spawn_exec_slots[i].args,
					     sizeof(spawn_exec_slots[i].args), envp, &off,
					     &spawn_exec_slots[i].envc) < 0)) {
				atomic_clear(&spawn_exec_slots[i].used);
				return -E2BIG;
			}
			return (int)i;
		}
	}

	return -EAGAIN;
}
#endif /* CONFIG_POSIX_EXEC_LLEXT */

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

#ifdef CONFIG_POSIX_EXEC_LLEXT
static void spawn_exec_slot_release(int slot)
{
	if (slot >= 0) {
		atomic_clear(&spawn_exec_slots[slot].used);
	}
}
#else
static inline void spawn_exec_slot_release(int slot)
{
	ARG_UNUSED(slot);
}
#endif /* CONFIG_POSIX_EXEC_LLEXT */

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
	int slot = -1;

	if (path == NULL) {
		return ENOENT;
	}

	if (file_actions != NULL) {
		if (spawn_file_actions_stage(file_actions, acts, file_actions->num) < 0) {
			return E2BIG;
		}
		args.fd_actions = acts;
		args.fd_actions_len = file_actions->num;
	}

	img = posix_spawn_image_lookup(path);
	if ((img != NULL) && (img->entry != NULL)) {
		args.entry = img->entry;
		/* the vectors are copied into the child */
		args.argv = argv;
		args.envp = envp;
	} else {
#ifdef CONFIG_POSIX_EXEC_LLEXT
		/* not a prelinked image: the child loads it from the file system */
		slot = spawn_exec_slot_claim(path, argv, envp);
		if (slot < 0) {
			if (slot == -ENAMETOOLONG) {
				return ENAMETOOLONG;
			}
			return (slot == -E2BIG) ? E2BIG : EAGAIN;
		}
		args.entry = spawn_exec_trampoline;
		args.p3 = (void *)(size_t)slot;
#else
		return ENOENT;
#endif /* CONFIG_POSIX_EXEC_LLEXT */
	}

	args.flags = SYS_CLONE_PAUSED;
	args.p1 = (void *)argv;
	args.p2 = (void *)envp;
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
		spawn_exec_slot_release(slot);
		return (ret == -EINVAL) ? EINVAL : EAGAIN;
	}

	/* the child is stopped: apply attributes, then start it */
	if (attrp != NULL) {
		if ((attrp->flags & POSIX_SPAWN_SETPGROUP) != 0) {
			/* pgroup 0 starts a new group led by the child (POSIX) */
			k_pgrp_t grp = (attrp->pgroup == 0) ? NULL
							    : sys_pgrp_find((int)attrp->pgroup);

			if (((attrp->pgroup != 0) && (grp == NULL)) ||
			    (sys_setpgid(child, grp) < 0)) {
				k_thread_abort(child);
				spawn_exec_slot_release(slot);
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
