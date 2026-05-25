/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

static void *worker(void *arg)
{
	printf("Hello World! %s from pthread_t %p\n", CONFIG_BOARD_TARGET,
	       (void *)(uintptr_t)pthread_self());

	return (void *)(intptr_t)42;
}

int main(void)
{
	void *res;
	pthread_t tid;

	pthread_create(&tid, NULL, worker, NULL);
	pthread_join(tid, &res);

	printf("pthread_t returned %d\n", (int)(intptr_t)res);

	return 0;
}
