/*
 * Copyright (c) 2021 Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include <zephyr/sys/heap_sort.h>

void qsort_r(void *base, size_t nmemb, size_t size,
	     int (*comp3)(const void *a, const void *b, void *arg), void *arg)
{
	if (nmemb <= 1) {
		return;
	}

	struct heap_sort_comp cmp = {
		.has3 = true,
		.arg = arg,
		{
			.comp3 = comp3
		}
	};

	heap_sort(base, nmemb, size, &cmp);
}
