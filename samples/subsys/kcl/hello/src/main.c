/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>
#include <zephyr/sys/thread.h>
#include <zephyr/xz/extract.h>

#define GUEST_PATH "/bin/hello"

/* the root: pages mapped as it fills, nothing of it in the image */
static struct fs_mount_t root_mnt = {
	.type = FS_TMPFS,
	.mnt_point = "/",
};

/* the guest, stripped and compressed; inflated into the root at boot */
static const uint8_t guest_xz[] __aligned(8) = {
#include "hello.inc"
};

static int spawn(k_pid_t *child, const char *path, char *const argv[], char *const envp[])
{
	struct sys_clone_args args = {
		.flags = SYS_CLONE_PAUSED,
		.prio = k_thread_priority_get(k_current_get()),
		.options = K_USER,
		.argv = argv,
		.envp = envp,
	};
	int ret;

	ret = sys_clone(&args, child);
	if (ret < 0) {
		return ret;
	}
	/* a Linux executable is loaded into the paused leader like any image */
	ret = sys_exec_load(*child, path);
	if (ret < 0) {
		k_thread_abort(*child);
		(void)k_waitpid(*child, NULL, NULL, 0, K_FOREVER);
		return ret;
	}
	k_thread_start(*child);

	return 0;
}

int main(void)
{
	static char *const argv[] = {GUEST_PATH, "from", "zephyr", NULL};
	static char *const envp[] = {"HOME=/", "LANG=C", NULL};
	int status = 0;
	k_pid_t pid;
	long n;
	int ret;

	ret = fs_mount(&root_mnt);
	if (ret < 0) {
		printf("mount failed: %d\n", ret);
		return ret;
	}
	(void)fs_mkdir("/bin");
	n = xz_extract(guest_xz, sizeof(guest_xz), GUEST_PATH);
	if (n < 0) {
		printf("extracting %s failed: %ld\n", GUEST_PATH, n);
		return (int)n;
	}
	printf("running %s (%zu bytes compressed, %ld bytes)\n", GUEST_PATH, sizeof(guest_xz), n);

	ret = spawn(&pid, GUEST_PATH, argv, envp);
	if (ret < 0) {
		printf("spawning %s failed: %d\n", GUEST_PATH, ret);
		return ret;
	}
	ret = k_waitpid(pid, NULL, &status, 0, K_FOREVER);
	if (ret < 0) {
		printf("waiting for %s failed: %d\n", GUEST_PATH, ret);
		return ret;
	}
	if (K_WIFEXITED(status)) {
		printf("%s exited with status %d\n", GUEST_PATH, K_WEXITSTATUS(status));
	} else if (K_WIFSIGNALED(status)) {
		printf("%s killed by signal %d\n", GUEST_PATH, K_WTERMSIG(status));
	}
	printf("kcl linux hello sample complete\n");

	return 0;
}
