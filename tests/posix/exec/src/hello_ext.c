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

	return 1;
}
LL_EXTENSION_SYMBOL(main);
