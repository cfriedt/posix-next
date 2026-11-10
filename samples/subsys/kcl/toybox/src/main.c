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

#define TOYBOX_PATH "/bin/toybox"

/* the root: pages mapped as it fills, nothing of it in the image */
static struct fs_mount_t root_mnt = {
	.type = FS_TMPFS,
	.mnt_point = "/",
};

/* toybox, stripped and compressed; inflated into the root at boot */
static const uint8_t toybox_xz[] __aligned(8) = {
#include "toybox.inc"
};

static const unsigned char demo_script[] = {
#include "demo_sh.inc"
};

static int write_file(const char *path, const void *data, size_t len)
{
	struct fs_file_t f;
	int ret;

	fs_file_t_init(&f);
	ret = fs_open(&f, path, FS_O_CREATE | FS_O_WRITE);
	if (ret < 0) {
		printf("creating %s failed: %d\n", path, ret);
		return ret;
	}
	for (size_t off = 0; off < len;) {
		ssize_t n = fs_write(&f, (const uint8_t *)data + off, MIN(1024, len - off));

		if (n <= 0) {
			printf("writing %s at %zu failed: %zd\n", path, off, n);
			ret = -1;
			break;
		}
		off += n;
	}
	(void)fs_close(&f);

	return (ret < 0) ? ret : 0;
}

/* toybox dispatches on the name it was invoked by: /bin/<applet> -> toybox */
static const char *const applets[] = {
#include "toybox_commands.inc"
};

static int populate(void)
{
	static const char passwd[] = "root:x:0:0:root:/root:/bin/sh\n";
	static const char group[] = "root:x:0:\n";
	static const char motd[] = "Hello from toybox for Linux on Zephyr!\n";
	static const char hostname[] = "zephyr\n";
	static const char hosts[] = "127.0.0.1 localhost\n::1 localhost ip6-localhost\n";
	static const char nsswitch[] = "hosts: files\n";
	long n;
	int ret;

	ret = fs_mount(&root_mnt);
	if (ret < 0) {
		printf("mount failed: %d\n", ret);
		return ret;
	}
	(void)fs_mkdir("/bin");
	(void)fs_mkdir("/etc");
	(void)fs_mkdir("/tmp");
	(void)fs_mkdir("/root");
	if ((write_file("/etc/passwd", passwd, strlen(passwd)) < 0) ||
	    (write_file("/etc/group", group, strlen(group)) < 0) ||
	    (write_file("/etc/motd", motd, strlen(motd)) < 0) ||
	    (write_file("/etc/hostname", hostname, strlen(hostname)) < 0) ||
	    (write_file("/etc/hosts", hosts, strlen(hosts)) < 0) ||
	    (write_file("/etc/nsswitch.conf", nsswitch, strlen(nsswitch)) < 0) ||
	    (write_file("/root/demo.sh", demo_script, sizeof(demo_script)) < 0)) {
		return -1;
	}
	n = xz_extract(toybox_xz, sizeof(toybox_xz), TOYBOX_PATH);
	if (n < 0) {
		printf("extracting %s failed: %ld\n", TOYBOX_PATH, n);
		return (int)n;
	}
	printf("toybox for Linux: %zu bytes compressed, %ld at %s\n", sizeof(toybox_xz), n,
	       TOYBOX_PATH);
	for (size_t i = 0; i < ARRAY_SIZE(applets); i++) {
		char path[48];

		(void)snprintf(path, sizeof(path), "/bin/%s", applets[i]);
		ret = fs_symlink("toybox", path);
		if (ret < 0) {
			printf("linking %s failed: %d\n", path, ret);
			return ret;
		}
	}
	printf("%zu applets linked in /bin\n", ARRAY_SIZE(applets));

	return 0;
}

/* the shell starts with nothing blocked, as it would on Linux */
static const struct k_sig_set nothing_blocked;

static int spawn(k_pid_t *child, const char *path, char *const argv[], char *const envp[])
{
	struct sys_clone_args args = {
		.flags = SYS_CLONE_PAUSED,
		.prio = k_thread_priority_get(k_current_get()),
		.options = K_USER,
		.sigmask = &nothing_blocked,
		.argv = argv,
		.envp = envp,
	};
	int ret;

	ret = sys_clone(&args, child);
	if (ret < 0) {
		return ret;
	}
	/* the image is loaded into the paused leader, then it is released */
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
	static char *const envp[] = {"PATH=/bin", "HOME=/root", "USER=root", "TERM=vt100", NULL};
	char *const interactive[] = {"sh", "-i", NULL};
	char *const scripted[] = {"sh", "-c", CONFIG_KCL_LINUX_TOYBOX_COMMAND, NULL};
	char *const *argv = (CONFIG_KCL_LINUX_TOYBOX_COMMAND[0] != '\0') ? scripted : interactive;
	int status = 0;
	k_pid_t pid;
	int ret;

	if (populate() < 0) {
		return -1;
	}
	printf("running sh%s\n", (argv == scripted) ? " -c" : " interactively");

	ret = spawn(&pid, "/bin/sh", argv, envp);
	if (ret < 0) {
		printf("spawning /bin/sh failed: %d\n", ret);
		return ret;
	}
	if (IS_ENABLED(CONFIG_KCL_LINUX_TOYBOX_PROMPT_LINE) && (argv == interactive)) {
		k_sleep(K_SECONDS(5));
		printf("toybox shell is up\n");
	}
	ret = k_waitpid(pid, NULL, &status, 0, K_FOREVER);
	if (ret < 0) {
		printf("waiting for sh failed: %d\n", ret);
		return ret;
	}
	if (K_WIFEXITED(status)) {
		printf("sh exited with status %d\n", K_WEXITSTATUS(status));
	} else if (K_WIFSIGNALED(status)) {
		printf("sh killed by signal %d\n", K_WTERMSIG(status));
	}
	printf("kcl linux toybox sample complete\n");

	return 0;
}
