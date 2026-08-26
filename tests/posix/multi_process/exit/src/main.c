/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * Exercises the ISO C exit sequence, which cannot run inside a ztest binary:
 * exit() must run atexit() handlers in reverse registration order. Output
 * order is asserted by the console harness.
 */

static void first_registered(void)
{
	printf("atexit:last\n");
}

static void second_registered(void)
{
	printf("atexit:second\n");
}

static void third_registered(void)
{
	printf("atexit:first\n");
}

int main(void)
{
	if (atexit(first_registered) != 0 || atexit(second_registered) != 0 ||
	    atexit(third_registered) != 0) {
		printf("atexit:registration failed\n");
		return 1;
	}

	printf("app:exiting\n");

	exit(0);

	return 0;
}
