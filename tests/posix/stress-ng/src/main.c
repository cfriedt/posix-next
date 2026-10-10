/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stress_ng.h>

#include <zephyr/fs/fs.h>
#include <zephyr/ztest.h>

static struct fs_mount_t tmp_mnt = {
	.type = FS_TMPFS,
	.mnt_point = CONFIG_STRESS_NG_TEMP_PATH,
};

static void *stress_ng_setup(void)
{
	zassert_ok(fs_mount(&tmp_mnt));

	return NULL;
}

/* one test per stressor built in: run it once, with verification, for the configured time */
static void run_stressor(const char *name)
{
	const struct stress_ng_opts opts = {
		.timeout_s = CONFIG_TEST_STRESS_NG_TIMEOUT,
	};
	int rc = stress_ng_run(name, &opts);

	switch (rc) {
	case STRESS_NG_EXIT_SUCCESS:
		break;
	case STRESS_NG_EXIT_NOT_IMPLEMENTED:
	case STRESS_NG_EXIT_NO_RESOURCE:
		ztest_test_skip();
		break;
	default:
		zassert_equal(rc, STRESS_NG_EXIT_SUCCESS, "%s: %s", name, stress_ng_strexit(rc));
		break;
	}
}

#define STRESS_NG_STRESSOR(sym, name)                                                              \
	ZTEST(stress_ng, test_##sym)                                                               \
	{                                                                                          \
		run_stressor(name);                                                                \
	}
#include <stress_ng_stressors.h>
#undef STRESS_NG_STRESSOR

ZTEST_SUITE(stress_ng, NULL, stress_ng_setup, NULL, NULL, NULL);
