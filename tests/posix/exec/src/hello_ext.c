/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/llext/symbol.h>

/*
 * The executable-image ABI: an exec'd extension exports main(), and its
 * return value becomes the process's exit status.
 */
int main(int argc, char **argv, char **envp)
{
	(void)envp;

	if ((argc == 2) && (argv[1][0] == 'x')) {
		return 42;
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
