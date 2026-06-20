/*
 * Copyright (c) 2024 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <grp.h>
#include <pwd.h>

#include <zephyr/ztest.h>

ZTEST(posix_system_database, test_getpwnam)
{
	struct passwd *result;

	{
		/* degenerate cases */
#ifndef CONFIG_NATIVE_LIBC
		errno = 0;
		zexpect_is_null(getpwnam(NULL));
		zexpect_equal(errno, 0);
#endif

#ifndef CONFIG_NATIVE_LIBC
		/* No guarantee that any specific user exists in Linux */

		/* user is not found in /etc/passwd */
		errno = 0;
		zexpect_is_null(getpwnam("nobody"));
		zexpect_equal(errno, 0, "expected errno to be zero, not %d", errno);
#endif
	}

	result = getpwnam("root");
	zassert_not_null(result, "getpwnam(\"root\") failed: %d", errno);
	zexpect_str_equal(result->pw_name, "root");
	zexpect_equal(result->pw_uid, 0);
	zexpect_equal(result->pw_gid, 0);
#ifndef CONFIG_NATIVE_LIBC
	/* root's home directory and shell can vary sometimes in Linux */

	zexpect_str_equal(result->pw_dir, "/root");
	zexpect_str_equal(result->pw_shell, "/bin/sh");
#endif

#ifndef CONFIG_NATIVE_LIBC
	/* No guarantee that any specific user exists in Linux */

	result = getpwnam("user");
	zassert_not_null(result, "getpwnam(\"user\") failed: %d", errno);
	zexpect_str_equal(result->pw_name, "user");
	zexpect_equal(result->pw_uid, 1000);
	zexpect_equal(result->pw_gid, 1000);
	zexpect_str_equal(result->pw_dir, "/home/user");
	zexpect_str_equal(result->pw_shell, "/bin/sh");
#endif
}

ZTEST(posix_system_database, test_getpwuid)
{
	struct passwd *result;

	{
		/* degenerate cases */

#ifndef CONFIG_NATIVE_LIBC
		/* No guarantee that any specific user exists in Linux */

		/* user is not found in /etc/passwd */
		errno = 0;
		zexpect_is_null(getpwuid(1001));
		zexpect_equal(errno, 0, "expected errno to be zero, not %d", errno);
#endif
	}

	result = getpwuid(0);
	zassert_not_null(result, "getpwuid(0) failed: %d", errno);
	zexpect_str_equal(result->pw_name, "root");
	zexpect_equal(result->pw_uid, 0);
	zexpect_equal(result->pw_gid, 0);
#ifndef CONFIG_NATIVE_LIBC
	/* root's home directory and shell can vary sometimes in Linux */

	zexpect_str_equal(result->pw_dir, "/root");
	zexpect_str_equal(result->pw_shell, "/bin/sh");
#endif

#ifndef CONFIG_NATIVE_LIBC
	/* No guarantee that any specific user exists in Linux */

	result = getpwuid(1000);
	zassert_not_null(result, "getpwuid(1000) failed: %d", errno);
	zexpect_str_equal(result->pw_name, "user");
	zexpect_equal(result->pw_uid, 1000);
	zexpect_equal(result->pw_gid, 1000);
	zexpect_str_equal(result->pw_dir, "/home/user");
	zexpect_str_equal(result->pw_shell, "/bin/sh");
#endif
}

#ifndef CONFIG_NATIVE_LIBC
static const char *const members[] = {
	"staff",
	"admin",
};
#endif

ZTEST(posix_system_database, test_getgrnam)
{
	struct group *result;

	{
#ifndef CONFIG_NATIVE_LIBC
		/* No guarantee that any specific user exists in Linux */

		/* degenerate cases */
		errno = 0;
		zexpect_is_null(getgrnam(NULL));
		zexpect_equal(errno, 0, "expected errno to be zero, not %d", errno);
#endif

		/* group is not found in /etc/group */
		errno = 0;
		zexpect_is_null(getgrnam("nobody"));
		zexpect_equal(errno, 0, "expected errno to be zero, not %d", errno);
	}

	result = getgrnam("root");
	zassert_not_null(result, "getgrnam(\"root\") failed: %d", errno);
	zexpect_str_equal(result->gr_name, "root");
	zexpect_equal(result->gr_gid, 0);
	zexpect_equal(result->gr_mem[0], NULL);

#ifndef CONFIG_NATIVE_LIBC
	/* No guarantee that any specific user exists in Linux */

	result = getgrnam("user");
	zassert_not_null(result, "getgrnam(\"user\") failed: %d", errno);
	zexpect_str_equal(result->gr_name, "user");
	zexpect_equal(result->gr_gid, 1000);
	ARRAY_FOR_EACH(members, i) {
		zexpect_str_equal(result->gr_mem[i], members[i],
				  "members[%d] (%s) does not match gr.gr_mem[%d] (%s)", (int)i,
				  members[i], (int)i, result->gr_mem[i]);
	}
	zexpect_equal(result->gr_mem[2], NULL);
#endif
}

ZTEST(posix_system_database, test_getgrgid)
{
	struct group *result;

	{
		/* degenerate cases */

#ifndef CONFIG_NATIVE_LIBC
		/* group is not found in /etc/group */
		errno = 0;
		zexpect_is_null(getgrgid(1001));
		zexpect_equal(errno, 0, "expected errno to be zero, not %d", errno);
#endif
	}

	result = getgrgid(0);
	zassert_not_null(result, "getgrgid(0) failed: %d", errno);
	zexpect_str_equal(result->gr_name, "root");
	zexpect_equal(result->gr_gid, 0);
	zexpect_equal(result->gr_mem[0], NULL);

#ifndef CONFIG_NATIVE_LIBC
	result = getgrgid(1000);
	zassert_not_null(result, "getgrgid(1000) failed: %d", errno);
	zexpect_str_equal(result->gr_name, "user");
	zexpect_equal(result->gr_gid, 1000);
	ARRAY_FOR_EACH(members, i) {
		zexpect_str_equal(result->gr_mem[i], members[i],
				  "members[%d] (%s) does not match gr.gr_mem[%d] (%s)", (int)i,
				  members[i], (int)i, result->gr_mem[i]);
	}
	zexpect_equal(result->gr_mem[2], NULL);
#endif
}

void *setup(void);
void teardown(void *arg);

ZTEST_SUITE(posix_system_database, NULL, setup, NULL, NULL, teardown);
