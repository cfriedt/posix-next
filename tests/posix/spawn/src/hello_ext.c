/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/llext/symbol.h>

/* a file system image: exports main(), whose return value is the exit status */
int main(int argc, char **argv, char **envp)
{
	if ((argc == 2) && (strcmp(argv[1], "x") == 0) && (envp[0] != NULL) &&
	    (strcmp(envp[0], "SPAWN=1") == 0) && (envp[1] == NULL)) {
		return 42;
	}

	return 1;
}
LL_EXTENSION_SYMBOL(main);
