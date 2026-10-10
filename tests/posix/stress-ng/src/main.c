/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

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

/* a stressor that could not clean up after itself must not starve the next one */
static void tmp_wipe(const char *dir, int depth)
{
	struct fs_dir_t d;
	struct fs_dirent entry;
	char path[CONFIG_ZVFS_PATH_MAX];

	fs_dir_t_init(&d);
	if ((depth > 8) || (fs_opendir(&d, dir) != 0)) {
		return;
	}
	while ((fs_readdir(&d, &entry) == 0) && (entry.name[0] != '\0')) {
		if (snprintf(path, sizeof(path), "%s/%s", dir, entry.name) >= (int)sizeof(path)) {
			continue;
		}
		if (entry.type == FS_DIR_ENTRY_DIR) {
			tmp_wipe(path, depth + 1);
		}
		(void)fs_unlink(path);
	}
	(void)fs_closedir(&d);
}

static void stress_ng_after(void *fixture)
{
	ARG_UNUSED(fixture);
	tmp_wipe(CONFIG_STRESS_NG_TEMP_PATH, 0);
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
	ZTEST_USER(stress_ng, test_##sym)                                                          \
	{                                                                                          \
		run_stressor(name);                                                                \
	}
#include <stress_ng_stressors.h>
#undef STRESS_NG_STRESSOR

ZTEST_SUITE(stress_ng, NULL, stress_ng_setup, NULL, stress_ng_after, NULL);
