/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "_main.h"

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

static const struct posix_threads_base_module modules[] = {
	{
		.setup = pthread_signal_setup,
	},
	{
		.before = pthread_attr_before,
		.after = pthread_attr_after,
	},
	{
		.before = mutex_before,
	},
};

static void *setup(void)
{
	ARRAY_FOR_EACH(modules, i) {
		if (modules[i].setup != NULL) {
			modules[i].setup();
		}
	}

	return NULL;
}

static void before(void *fixture)
{
	ARRAY_FOR_EACH(modules, i) {
		if (modules[i].before != NULL) {
			modules[i].before(fixture);
		}
	}
}

static void after(void *fixture)
{
	ARRAY_FOR_EACH(modules, i) {
		if (modules[i].after != NULL) {
			modules[i].after(fixture);
		}
	}
}

static void teardown(void *fixture)
{
	for (size_t i = ARRAY_SIZE(modules); i > 0; i--) {
		if (modules[i - 1].teardown != NULL) {
			modules[i - 1].teardown(fixture);
		}
	}
}

ZTEST_SUITE(posix_threads_base, NULL, setup, before, after, teardown);
