/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zephyr/llext/symbol.h>

/* a file system image: exports main(), whose return value is the exit status */
int main(int argc, char **argv, char **envp)
{
	if ((argc == 2) && (strcmp(argv[1], "x") == 0) && (envp[0] != NULL) &&
	    (strcmp(envp[0], "SPAWN=1") == 0) && (envp[1] == NULL)) {
		/* the process's own arena: allocate, use, and give back */
		char *blocks[8];

		for (size_t i = 0; i < 8; i++) {
			blocks[i] = malloc(64 + 16 * i);
			if (blocks[i] == NULL) {
				return 2;
			}
			memset(blocks[i], (int)i, 64 + 16 * i);
		}
		for (size_t i = 0; i < 8; i++) {
			if ((unsigned char)blocks[i][63] != i) {
				return 3;
			}
			free(blocks[i]);
		}
		return 42;
	}

	if ((argc == 2) && (strcmp(argv[1], "cwd") == 0)) {
		/* the parent's directory at spawn; changing it here leaves the parent's alone */
		char buf[16];

		if ((getcwd(buf, sizeof(buf)) == NULL) || (strcmp(buf, "/RAM:/wd") != 0)) {
			return 4;
		}
		if ((chdir("/") != 0) || (getcwd(buf, sizeof(buf)) == NULL) ||
		    (strcmp(buf, "/") != 0)) {
			return 5;
		}
		return 44;
	}

	return 1;
}
LL_EXTENSION_SYMBOL(main);
