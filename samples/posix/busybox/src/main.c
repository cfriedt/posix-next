/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <zephyr/app_memory/app_memdomain.h>
#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>

#define BUSYBOX_PATH "/bin/busybox"

static struct fs_mount_t root_mnt = {
	.type = FS_EXT2,
	.mnt_point = "/",
	.storage_dev = (void *)"RAM",
};

static const uint8_t busybox_image[] __aligned(8) = {
#include "busybox.inc"
};

extern char **environ;

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
	/* one fs block at a time: a single 100 KiB+ fs_write() to ext2 has
	 * been seen to corrupt the file's first blocks
	 */
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

static int run(const char *path, char *const argv[])
{
	pid_t pid;
	int status = 0;
	int ret;

	printf("\n$ %s", argv[0]);
	for (int i = 1; argv[i] != NULL; i++) {
		printf(" %s", argv[i]);
	}
	printf("\n");

	ret = posix_spawn(&pid, path, NULL, NULL, argv, environ);
	if (ret != 0) {
		printf("posix_spawn() failed: %d\n", ret);
		return -1;
	}

	if (waitpid(pid, &status, 0) < 0) {
		printf("waitpid() failed: %d\n", errno);
		return -1;
	}

	if (WIFEXITED(status) && (WEXITSTATUS(status) != 0)) {
		printf("exit status: %d\n", WEXITSTATUS(status));
	}

	return status;
}

#define RUN(...) run(BUSYBOX_PATH, (char *const[]){"busybox", __VA_ARGS__, NULL})
#define RUN_AS(name, ...) run("/bin/" name, (char *const[]){name, __VA_ARGS__, NULL})

int main(void)
{
#ifdef CONFIG_USERSPACE
	/* what busybox's processes touch of the system's shared state: directory streams
	 * and the user and group databases
	 */
	extern struct k_mem_partition zvfs_dir_partition;
	extern struct k_mem_partition posix_system_database_partition;

	(void)k_mem_domain_add_partition(&k_mem_domain_default, &zvfs_dir_partition);
	(void)k_mem_domain_add_partition(&k_mem_domain_default, &posix_system_database_partition);
#endif /* CONFIG_USERSPACE */
	static const char passwd[] = "root:x:0:0:root:/root:/bin/sh\n";
	static const char group[] = "root:x:0:\n";
	static const char motd[] = "Hello from busybox on Zephyr!\n";
	int ret;

	ret = fs_mkfs(FS_EXT2, (uintptr_t)"RAM", NULL, 0);
	if (ret < 0) {
		printf("mkfs failed: %d\n", ret);
		return ret;
	}

	ret = fs_mount(&root_mnt);
	if (ret < 0) {
		printf("mount failed: %d\n", ret);
		return ret;
	}

	(void)fs_mkdir("/bin");
	(void)fs_mkdir("/etc");
	(void)fs_mkdir("/root");

	if (write_file("/etc/passwd", passwd, strlen(passwd)) < 0 ||
	    write_file("/etc/group", group, strlen(group)) < 0 ||
	    write_file("/etc/motd", motd, strlen(motd)) < 0 ||
	    write_file("/bin/busybox", busybox_image, sizeof(busybox_image)) < 0) {
		return -1;
	}

	RUN("uname", "-a");
	RUN("echo", "hello", "world");
	RUN("cat", "/etc/motd");
	RUN("mkdir", "/tmp");
	RUN("pwd");
	RUN("ls", "-l", "/etc");

	/* symbolic links on ext2: create, read back, follow */
	RUN("ln", "-s", "motd", "/etc/motd.lnk");
	RUN("readlink", "/etc/motd.lnk");
	RUN("cat", "/etc/motd.lnk");

	/* the classic busybox install: every applet becomes a /bin symlink to
	 * the binary, exec follows the link, and busybox dispatches on argv[0]
	 */
	RUN("--install", "-s", "/bin");
	RUN("ping", "-c", "1", "127.0.0.1");
	RUN("ping6", "-c", "1", "::1");
#ifdef CONFIG_NET_CONFIG_SETTINGS
	/* a real interface is configured: reach past the loopback */
	RUN("ping", "-c", "1", CONFIG_NET_CONFIG_MY_IPV4_GW);
#endif
	RUN_AS("ls", "-l", "/bin");

	/* hush: builtins, variables, globbing, and control flow run in the
	 * shell process itself; pipelines and external commands need fork()
	 * (user-mode processes with CONFIG_PROCESS_VM) and are not yet part
	 * of the demo
	 */
	static const char init_sh[] = "echo hush: hello from $0\n"
				      "uname -m\n"
				      "for f in /etc/*; do echo saw $f; done\n"
				      "ls /etc | cat\n"
				      "printf 'hex of 42 is %x\\n' 42\n"
				      "x=zephyr\n"
				      "case $x in z*) echo case matched $x ;; esac\n";

	if (write_file("/etc/init.sh", init_sh, strlen(init_sh)) == 0) {
		RUN("sh", "-c", "echo hello from hush | cat");
		RUN("sh", "/etc/init.sh");
	}

	printf("\nbusybox sample complete\n");

	/* and finally: a shell on the console. Ctrl-D or 'exit' leaves it. */
	printf("\nstarting an interactive shell\n");
	RUN("sh");
	printf("goodbye\n");

	return 0;
}
