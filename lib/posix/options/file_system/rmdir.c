/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

/**
 * @brief Remove a directory.
 *
 * See IEEE 1003.1
 */
int rmdir(const char *path)
{
	return unlink(path);
}
