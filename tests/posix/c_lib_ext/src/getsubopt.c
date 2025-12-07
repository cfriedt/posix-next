/*
 * Copyright (c) 2025 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include <stdlib.h>

ZTEST(posix_c_lib_ext, test_getsubopt)
{
	char *value;
	char *option;
	char buf[32];

	enum opte {
		RO_OPTION,
		RW_OPTION,
		READ_SIZE_OPTION,
		WRITE_SIZE_OPTION,
		EMBEDDED_EQ_OPTION,
	};

	static const char *const key_list[] = {
		[RO_OPTION] = "ro",
		[RW_OPTION] = "rw",
		[READ_SIZE_OPTION] = "rsize",
		[WRITE_SIZE_OPTION] = "wsize",
		[EMBEDDED_EQ_OPTION] = "equal",
		NULL,
	};

	static const char *const empty_key_list[] = {
		NULL,
	};

	/* empty options string */
	memset(buf, 0, sizeof(buf));
	option = buf;
	value = (char *)0x4242;
	zexpect_equal(-1, getsubopt(&option, (char **)key_list, &value));
	/* empty key list */
	strcpy(buf, "ro,rsize=512,equal=1=2,rw");
	option = buf;
	value = (char *)0x4242;
	zexpect_equal(-1, getsubopt(&option, (char **)empty_key_list, &value));

	strcpy(buf, "ro,rsize=512,equal=1=2,rw");
	option = buf;
	value = (char *)0x4242;
	zexpect_equal(RO_OPTION, getsubopt(&option, (char **)key_list, &value));
	zexpect_equal(option, &buf[strlen("ro,")]);

	zexpect_equal(READ_SIZE_OPTION, getsubopt(&option, (char **)key_list, &value));
	zexpect_equal(option, &buf[strlen("ro,rsize=512,")]);
	zexpect_not_null(value);
	if (value != NULL) {
		zexpect_str_equal(value, "512");
	}

	value = (char *)0x4242;
	zexpect_equal(EMBEDDED_EQ_OPTION, getsubopt(&option, (char **)key_list, &value));
	zexpect_equal(option, &buf[strlen("ro,rsize=512,equal=1=2,")]);
	zexpect_not_null(value);
	if (value != NULL) {
		zexpect_str_equal(value, "1=2");
	}

	value = (char *)0x4242;
	zexpect_equal(RW_OPTION, getsubopt(&option, (char **)key_list, &value));
	zexpect_equal(option, &buf[strlen("ro,rsize=512,equal=1=2,rw")]);

	value = (char *)0x4242;
	zexpect_equal(getsubopt(&option, (char **)key_list, &value), -1);

	strcpy(buf, "oops");
	option = buf;
	value = (char *)0x4242;
	zexpect_equal(getsubopt(&option, (char **)key_list, &value), -1);

	/* some corner cases */
	strcpy(buf, ",rsize=,");
	option = buf;
	value = (char *)0x4242;
	zexpect_equal(-1, getsubopt(&option, (char **)key_list, &value));
	zexpect_equal(option, &buf[strlen(",")]);

	value = (char *)0x4242;
	zexpect_equal(READ_SIZE_OPTION, getsubopt(&option, (char **)key_list, &value));
	zexpect_equal(option, &buf[strlen(",rsize=,")]);
	zexpect_not_null(value);
	if (value != NULL) {
		zexpect_str_equal(value, "");
	}
}
