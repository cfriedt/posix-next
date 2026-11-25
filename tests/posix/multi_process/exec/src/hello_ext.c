/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* the image is built without the application's feature test macros */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <zephyr/llext/symbol.h>

/*
 * The executable-image ABI: an exec'd extension exports main(), and its
 * return value becomes the process's exit status. An environ it exports is
 * bound to the environment it is entered with.
 */
char **environ;
LL_EXTENSION_SYMBOL(environ);

int main(int argc, char **argv, char **envp)
{
	if ((argc == 2) && (argv[1][0] == 'x')) {
		/* errno is the running thread's: the image reaches it through the base image */
		errno = 0;
		if ((close(-1) != -1) || (errno != EBADF)) {
			return 45;
		}
		return 42;
	}
	if ((argc >= 2) && (argv[1][0] == 'n')) {
		if ((environ != envp) || (envp[0] == NULL) || (envp[1] != NULL) ||
		    (strcmp(envp[0], "HELLO=world") != 0)) {
			return 50;
		}
		/* an image process with C library state of its own has an environment store */
		if (argc == 3) {
			const char *val = getenv("HELLO");

			if ((val == NULL) || (strcmp(val, "world") != 0)) {
				return 51;
			}
			if (setenv("ADDED", "1", 1) != 0) {
				return 52;
			}
			/* the store is what environ names */
			if ((environ[0] == NULL) || (environ[1] == NULL) ||
			    (strcmp(environ[1], "ADDED=1") != 0) || (environ[2] != NULL)) {
				return 53;
			}
		}
		return 44;
	}
	if ((argc == 2) && (argv[1][0] == 'e')) {
		/* die without returning: the reaper must unload the image */
		_exit(43);
	}
	if ((argc == 2) && (argv[1][0] == 'c')) {
		/* chain-exec a second image: the successor unloads this one */
		char *const chain_argv[] = {"hello2", "e", NULL};

		(void)execve("/RAM:/hello2.llext", chain_argv, NULL);
		return 4;
	}

	return 1;
}
LL_EXTENSION_SYMBOL(main);
