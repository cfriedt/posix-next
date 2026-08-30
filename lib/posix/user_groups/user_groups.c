/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <zephyr/toolchain.h>

/* Zephyr is a single-user system: the only identity is the privileged user */

gid_t getegid(void)
{
	return 0;
}

uid_t geteuid(void)
{
	return 0;
}

gid_t getgid(void)
{
	return 0;
}

uid_t getuid(void)
{
	return 0;
}

int getgroups(int gidsetsize, gid_t grouplist[])
{
	ARG_UNUSED(grouplist);

	if (gidsetsize < 0) {
		errno = EINVAL;
		return -1;
	}

	/* the supplementary group set is empty */
	return 0;
}

char *getlogin(void)
{
	static const char name[] = "root";

	return (char *)name;
}

int setegid(gid_t gid)
{
	if (gid != 0) {
		errno = EPERM;
		return -1;
	}

	return 0;
}

int seteuid(uid_t uid)
{
	if (uid != 0) {
		errno = EPERM;
		return -1;
	}

	return 0;
}

int setgid(gid_t gid)
{
	if (gid != 0) {
		errno = EPERM;
		return -1;
	}

	return 0;
}

int setuid(uid_t uid)
{
	if (uid != 0) {
		errno = EPERM;
		return -1;
	}

	return 0;
}
