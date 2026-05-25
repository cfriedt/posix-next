/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <stdio.h>

#include <zephyr/kernel.h>

void worker_entry(void *p1, void *p2, void *p3)
{
	printf("Hello World! %s from k_thread %p\n", CONFIG_BOARD_TARGET, k_current_get());

	/* Zephyr's native threading API still does not support a direct return value */
	k_thread_result_set(INT_TO_POINTER(42));
}

K_THREAD_DEFINE(kth, 2048, worker_entry, NULL, NULL, NULL, K_LOWEST_APPLICATION_THREAD_PRIO,
		0, SYS_FOREVER_MS);

int main(void)
{
	void *res;
	pthread_t th = (pthread_t)(uintptr_t)kth;

	k_thread_start(kth);
	pthread_join(th, &res);

	printf("k_thread returned %d\n", POINTER_TO_INT(res));

	return 0;
}
