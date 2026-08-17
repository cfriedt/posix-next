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
#include <zephyr/llext/llext.h>
#include <zephyr/llext/fs_loader.h>

#include "multi_process_internal.h"

/*
 * Which loaded extension backs which exec'd process. An image cannot unload
 * itself (its caller executes from the extension), so images terminated by
 * _exit() or a signal are unloaded by the reaper when the process is
 * collected; a chain-exec'ing image is unloaded by its successor past exec's
 * point of no return. Auto-reaped children (parent ignoring SIGCHLD) hold
 * their slot until the table sweep notices the pid retired.
 */
static struct {
	k_pid_t pid;
	int num;
	struct llext *ext;
} exec_images[CONFIG_POSIX_EXEC_LLEXT_MAX];
static struct k_spinlock exec_images_lock;

static struct llext *exec_image_detach(k_pid_t pid, struct llext *skip)
{
	struct llext *ext = NULL;

	K_SPINLOCK(&exec_images_lock) {
		for (size_t i = 0; i < ARRAY_SIZE(exec_images); i++) {
			if ((exec_images[i].ext != NULL) && (exec_images[i].ext != skip) &&
			    (exec_images[i].pid == pid)) {
				ext = exec_images[i].ext;
				exec_images[i].ext = NULL;
				K_SPINLOCK_BREAK;
			}
		}
	}

	return ext;
}

void z_posix_exec_llext_reap(k_pid_t reaped)
{
	struct llext *ext;

	/* both slots of a process killed mid-exec unload here */
	while ((ext = exec_image_detach(reaped, NULL)) != NULL) {
		(void)llext_unload(&ext);
	}
}

static int exec_image_register(struct llext *ext, k_pid_t self)
{
	int num = sys_process_id(self);

	for (int pass = 0; pass < 2; pass++) {
		bool claimed = false;

		K_SPINLOCK(&exec_images_lock) {
			for (size_t i = 0; i < ARRAY_SIZE(exec_images); i++) {
				if (exec_images[i].ext == NULL) {
					exec_images[i].pid = self;
					exec_images[i].num = num;
					exec_images[i].ext = ext;
					claimed = true;
					K_SPINLOCK_BREAK;
				}
			}
		}
		if (claimed) {
			return 0;
		}

		/* full: sweep slots whose pid retired without a waitpid() */
		for (size_t i = 0; (pass == 0) && (i < ARRAY_SIZE(exec_images)); i++) {
			struct llext *stale = NULL;
			k_pid_t pid = exec_images[i].pid;
			int expect = exec_images[i].num;

			if ((exec_images[i].ext == NULL) || (sys_process_id(pid) == expect)) {
				continue;
			}
			K_SPINLOCK(&exec_images_lock) {
				if ((exec_images[i].ext != NULL) && (exec_images[i].pid == pid) &&
				    (exec_images[i].num == expect)) {
					stale = exec_images[i].ext;
					exec_images[i].ext = NULL;
				}
			}
			if (stale != NULL) {
				(void)llext_unload(&stale);
			}
		}
	}

	return -ENOMEM;
}

int z_posix_exec_llext(const char *path, char *const argv[], char *const envp[])
{
	struct llext_fs_loader fldr = LLEXT_FS_LOADER(path);
	struct llext_load_param param = LLEXT_LOAD_PARAM_DEFAULT;
	struct llext *ext = NULL;
	struct llext *prior;
	int (*ext_main)(int argc, char **argv, char **envp);
	const char *name;
	int argc = 0;
	int ret;

	/* the extension's name is the path's last component */
	name = strrchr(path, '/');
	name = (name == NULL) ? path : (name + 1);

	ret = llext_load(&fldr.loader, name, &ext, &param);
	if (ret != 0) {
		errno = ENOENT;
		return -1;
	}

	*(const void **)&ext_main = llext_find_sym(&ext->exp_tab, "main");
	if (ext_main == NULL) {
		(void)llext_unload(&ext);
		/* a loadable object that is not an executable image */
		errno = ENOEXEC;
		return -1;
	}

	ret = exec_image_register(ext, k_getpid());
	if (ret != 0) {
		(void)llext_unload(&ext);
		errno = ENOMEM;
		return -1;
	}

	/*
	 * Point of no return: nothing below the current frame runs again, so
	 * a chain-exec'ing caller's own extension can be unloaded now.
	 */
	prior = exec_image_detach(k_getpid(), ext);
	if (prior != NULL) {
		(void)llext_unload(&prior);
	}

	z_posix_exec_prepare();

	while ((argv != NULL) && (argv[argc] != NULL)) {
		argc++;
	}

	/*
	 * The image ABI: the extension exports int main(int, char **, char **)
	 * and its return value becomes the process's exit status. The image
	 * runs on the calling thread's stack (the documented exec deviation).
	 * A normal return unloads here; _exit() or signal death leaves the
	 * unload to the reaper via z_posix_exec_llext_reap().
	 */
	ret = ext_main(argc, (char **)argv, (char **)envp);
	ext = exec_image_detach(k_getpid(), NULL);
	if (ext != NULL) {
		(void)llext_unload(&ext);
	}
	exit(ret);
}
