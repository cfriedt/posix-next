/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/* the guest: a plain Linux program, built with the host's C library */

#include <stdio.h>
#include <sys/utsname.h>
#include <unistd.h>

int main(int argc, char *argv[], char *envp[])
{
	struct utsname uts;

	printf("hello, world\n");
	printf("pid %d, %d argument(s):", (int)getpid(), argc);
	for (int i = 0; i < argc; i++) {
		printf(" %s", argv[i]);
	}
	printf("\n");
	for (char **e = envp; *e != NULL; e++) {
		printf("env: %s\n", *e);
	}
	if (uname(&uts) == 0) {
		printf("running on %s %s %s\n", uts.sysname, uts.release, uts.machine);
	}

	return 42;
}
