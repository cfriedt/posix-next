/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/llext/symbol.h>

/* a command that takes a CPU fault: a user-mode process is killed by SIGSEGV */
int main(int argc, char **argv, char **envp)
{
	volatile int *nowhere = NULL;

	(void)argc;
	(void)envp;
	printf("%s: storing through a null pointer\n", argv[0]);
	*nowhere = 0;

	return 0;
}
LL_EXTENSION_SYMBOL(main);
