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
#if defined(CONFIG_USERSPACE) && defined(CONFIG_MPU)
	struct k_mem_domain domain;
#endif
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
				/* K_SPINLOCK_BREAK is continue-based: it would resume
				 * this inner loop, not leave the locked block
				 */
				break;
			}
		}
	}

	return ext;
}

void z_posix_exec_llext_reap(k_pid_t reaped)
{
	struct llext *ext;

	if (k_is_user_context()) {
		/* table and loader are supervisor-side; the sweep reclaims later */
		return;
	}

	/* both slots of a process killed mid-exec unload here */
	while ((ext = exec_image_detach(reaped, NULL)) != NULL) {
		(void)llext_unload(&ext);
	}
}

static int exec_image_register(struct llext *ext, k_pid_t self)
{
	int num = sys_process_id(self);

	for (int pass = 0; pass < 2; pass++) {
		int claimed = -1;

		K_SPINLOCK(&exec_images_lock) {
			for (size_t i = 0; i < ARRAY_SIZE(exec_images); i++) {
				if (exec_images[i].ext == NULL) {
					exec_images[i].pid = self;
					exec_images[i].num = num;
					exec_images[i].ext = ext;
					claimed = (int)i;
					break;
				}
			}
		}
		if (claimed >= 0) {
			return claimed;
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

static void exec_image_release(int slot)
{
	K_SPINLOCK(&exec_images_lock) {
		exec_images[slot].ext = NULL;
	}
}

#if defined(CONFIG_USERSPACE) && defined(CONFIG_MPU)
/*
 * MPU parts map RAM execute-never when XIP: the image's RX partitions,
 * programmed as dynamic regions via the caller's memory domain, override the
 * static map. MMU parts need none of this - kernel mappings are executable.
 */
static int exec_domain_enter(int slot, struct llext *ext)
{
	int ret;

	k_mem_domain_init(&exec_images[slot].domain, 0, NULL);
	ret = llext_add_domain(ext, &exec_images[slot].domain);
	if (ret != 0) {
		return ret;
	}

	ret = k_mem_domain_add_thread(&exec_images[slot].domain, k_current_get());
	if (ret == 0) {
		/* domain changes program the MPU on context switch, so take one */
		k_sleep(K_TICKS(1));
	}

	return ret;
}
#else
static int exec_domain_enter(int slot, struct llext *ext)
{
	ARG_UNUSED(slot);
	ARG_UNUSED(ext);
	return 0;
}
#endif /* CONFIG_USERSPACE && CONFIG_MPU */

int z_posix_exec_llext(const char *path, char *const argv[], char *const envp[])
{
	struct llext_fs_loader fldr = LLEXT_FS_LOADER(path);
	struct llext_load_param param = LLEXT_LOAD_PARAM_DEFAULT;
	struct llext *ext = NULL;
	struct llext *prior;
	int (*ext_main)(int argc, char **argv, char **envp);
	const char *name;
	int slot;
	int ret;

	/* the extension's name is the path's last component */
	name = strrchr(path, '/');
	name = (name == NULL) ? path : (name + 1);

	/* a positive return is a use count: the image is already loaded and shared */
	ret = llext_load(&fldr.loader, name, &ext, &param);
	if (ret < 0) {
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

	if (z_posix_exec_args_check(argv, envp) != 0) {
		(void)llext_unload(&ext);
		return -1;
	}

	slot = exec_image_register(ext, k_getpid());
	if (slot < 0) {
		(void)llext_unload(&ext);
		errno = ENOMEM;
		return -1;
	}

	ret = exec_domain_enter(slot, ext);
	if (ret != 0) {
		exec_image_release(slot);
		(void)llext_unload(&ext);
		errno = ENOMEM;
		return -1;
	}

	struct z_posix_exec_run_args run = {
		.ext_main = ext_main,
		.argv = argv,
		.envp = envp,
	};

	/*
	 * Point of no return: nothing below the current frame runs again, so
	 * a chain-exec'ing caller's own extension can be unloaded now.
	 */
	prior = exec_image_detach(k_getpid(), ext);
	if (prior != NULL) {
		(void)llext_unload(&prior);
	}

	/*
	 * The image ABI: the extension exports int main(int, char **, char **)
	 * and its return value becomes the process's exit status; a normal
	 * return unloads the extension, _exit() or signal death leaves the
	 * unload to the reaper via z_posix_exec_llext_reap().
	 */
	z_posix_exec_run(&run);
	CODE_UNREACHABLE;
}
