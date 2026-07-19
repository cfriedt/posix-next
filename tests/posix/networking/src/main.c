/*
 * Copyright (c) 2024, Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

void *test_fs_setup(void);
void test_fs_teardown(void *arg);
void test_netdb_cleanup(void *arg);

ZTEST_SUITE(posix_networking, NULL, test_fs_setup, NULL, test_netdb_cleanup, test_fs_teardown);
