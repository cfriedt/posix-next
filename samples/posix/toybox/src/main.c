/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <spawn.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>

#define TOYBOX_PATH "/bin/toybox"

static struct fs_mount_t root_mnt = {
	.type = FS_EXT2,
	.mnt_point = "/",
	.storage_dev = (void *)"RAM",
};

static const uint8_t toybox_image[] __aligned(8) = {
#include "toybox.inc"
};

/* the commands the image provides: each is a /bin symlink to the binary */
static const char *const toybox_commands[] = {
#include "toybox_commands.inc"
};

/* the environment every spawned process starts with */
static char *const spawn_env[] = {"PATH=/bin", "HOME=/root", NULL};

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

static int spawn_wait(const char *path, char *const argv[])
{
	pid_t pid;
	int status = 0;
	int ret;

	ret = posix_spawn(&pid, path, NULL, NULL, argv, spawn_env);
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
	} else if (WIFSIGNALED(status)) {
		printf("killed by signal %d\n", WTERMSIG(status));
	}

	return status;
}

/* echo the command line the way a shell would, then run it */
static int run(const char *path, char *const argv[])
{
	printf("\n$ %s", argv[0]);
	for (int i = 1; argv[i] != NULL; i++) {
		printf(" %s", argv[i]);
	}
	printf("\n");

	return spawn_wait(path, argv);
}

/* toybox dispatches on argv[0]: "toybox CMD ..." or the command name itself */
#define RUN(...)          run(TOYBOX_PATH, (char *const[]){"toybox", __VA_ARGS__, NULL})
#define RUN_AS(name, ...) run("/bin/" name, (char *const[]){name, __VA_ARGS__, NULL})

/* the scripted tour: what the command set and the shell can do */
static void tour(void)
{
	/* no arguments: list the available commands */
	run(TOYBOX_PATH, (char *const[]){"toybox", NULL});
	RUN("uname", "-a");
	RUN("echo", "hello", "world");
	RUN("cat", "/etc/motd");
	RUN("mkdir", "/tmp/scratch");
	RUN("pwd");
	RUN("ls", "-l", "/etc");
	RUN("printf", "hex of %d is %x\\n", "42", "42");
	RUN("seq", "3");
	RUN("wc", "/etc/motd");

	/* symbolic links on ext2: create, read back, follow */
	RUN("ln", "-s", "motd", "/etc/motd.lnk");
	RUN("readlink", "/etc/motd.lnk");
	RUN("cat", "/etc/motd.lnk");

	/* a command run under its own name */
	RUN_AS("ls", "-l", "/bin/ls", "/bin/sh");
	RUN("sh", "-c", "ls /bin | wc -l");

	/* toysh: builtins, variables, globbing and control flow run in the
	 * shell process itself; external commands are spawned. Subshells and
	 * command substitution need fork() and are not yet part of the demo.
	 */
	static const char init_sh[] = "echo toysh: hello from $0\n"
				      "uname -m\n"
				      "for f in /etc/*; do echo saw $f; done\n"
				      "ls /etc | cat\n"
				      "printf 'hex of 42 is %x\\n' 42\n"
				      "x=zephyr\n"
				      "case $x in z*) echo case matched $x ;; esac\n";

	if (write_file("/etc/init.sh", init_sh, strlen(init_sh)) == 0) {
		RUN("sh", "-c", "echo hello from toysh | cat");
		RUN("sh", "/etc/init.sh");
	}

	RUN("ping", "-c", "1", "127.0.0.1");
	RUN("ping6", "-c", "1", "::1");
#ifdef CONFIG_PROCESS_VM
	/* subshells and command substitution fork() the shell */
	RUN("sh", "-c", "(echo subshell works); echo cmdsub is $(echo works)");
#endif
#ifdef CONFIG_NET_CONFIG_SETTINGS
	/* a real interface is configured: reach past the loopback */
	RUN("ping", "-c", "1", CONFIG_NET_CONFIG_MY_IPV4_GW);
#ifdef CONFIG_NET_CONFIG_NEED_IPV6
	RUN("ping6", "-c", "1", "fec0::2");
#endif
#ifdef CONFIG_DNS_RESOLVER
	/* a name, through the resolver; the echo itself needs the host's internet */
	RUN("ping", "-c", "1", "dns.google");
#endif
#endif

	printf("\ntoybox sample complete\n");
	printf("\nstarting an interactive shell\n");
}

int main(void)
{
#ifdef CONFIG_USERSPACE
	/* what toybox's processes touch of the system's shared state: directory streams
	 * and the user and group databases
	 */
	extern struct k_mem_partition zvfs_dir_partition;
	extern struct k_mem_partition posix_system_database_partition;

	(void)k_mem_domain_add_partition(&k_mem_domain_default, &zvfs_dir_partition);
	(void)k_mem_domain_add_partition(&k_mem_domain_default, &posix_system_database_partition);
#endif /* CONFIG_USERSPACE */
	static const char passwd[] = "root:x:0:0:root:/root:/bin/sh\n";
	static const char group[] = "root:x:0:\n";
	static const char motd[] = "Hello from toybox on Zephyr!\n";
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
	(void)fs_mkdir("/tmp");

	if (write_file("/etc/passwd", passwd, strlen(passwd)) < 0 ||
	    write_file("/etc/group", group, strlen(group)) < 0 ||
	    write_file("/etc/motd", motd, strlen(motd)) < 0 ||
	    write_file(TOYBOX_PATH, toybox_image, sizeof(toybox_image)) < 0) {
		return -1;
	}

	/* the usual install: each command is a /bin symlink to the binary,
	 * exec follows the link, and toybox dispatches on argv[0]
	 */
	for (size_t i = 0; i < ARRAY_SIZE(toybox_commands); i++) {
		char link[32];

		snprintf(link, sizeof(link), "/bin/%s", toybox_commands[i]);
		ret = fs_symlink("toybox", link);
		if (ret < 0) {
			printf("linking %s failed: %d\n", link, ret);
			return ret;
		}
	}
	printf("%zu commands linked in /bin\n", ARRAY_SIZE(toybox_commands));

	if (IS_ENABLED(CONFIG_SAMPLE_TOYBOX_TOUR)) {
		tour();
	}

	/* and finally: a shell on the console. Ctrl-D or 'exit' leaves it. */
	spawn_wait(TOYBOX_PATH, (char *const[]){"toybox", "sh", NULL});
	printf("goodbye\n");

	return 0;
}
