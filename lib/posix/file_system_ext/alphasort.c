/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>
#include <string.h>

int alphasort(const struct dirent **d1, const struct dirent **d2)
{
	/* single-locale environment: collation order is byte order */
	return strcmp((*d1)->d_name, (*d2)->d_name);
}
