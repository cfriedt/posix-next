/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

/*
 * Note1: the only portable way to restart scanning is to set optind to 1. However, that is
 * also not explicitly mentioned in the POSIX specification.
 *
 * https://pubs.opengroup.org/onlinepubs/9699919799/functions/getopt.html
 *
 * The specification does state that setting optind to zero before calling getopt results in
 * unspecified behavior:
 *
 * > If the application sets optind to zero before calling getopt(), the behavior is unspecified
 *
 * Note2: POSIX requires that argv[] is terminated by a NULL pointer.
 *
 * https://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html
 *
 * > The argv and environ arrays are each terminated by a null pointer. The null pointer
 * > terminating the argv array is not counted in argc.
 *
 * Of course, if it is assumed that argc is accurate, then the NULL terminator is not strictly
 * necessary for getopt to function.
 *
 * Note3: getopt is not required to be thread-safe.
 *
 * The implication is that external synchronization is required for multiple concurrent callers
 * in order to be pedantically conformant to the specification. However, if we relax extern
 * linking criteria (i.e. `extern int optind`) and simply declare getopt's global state variables
 * as TLS variables (i.e. `extern int __thread optind`), then getopt is thread safe.
 *
 * That approach is sound, but cannot be imposed on external C library implementations, and must
 * be explicitly stated as a deviation from POSIX (likely based on a Kconfig option).
 */

#if defined(CONFIG_NATIVE_LIBC)
/* glibc() use char *argv[], which breaks conformance */
#define GETOPT_ARGV_CAST char **
#else
/* POSIX requires the argv array to be of type char *const [] */
#define GETOPT_ARGV_CAST char *const *
#endif

#define TEST_MAX_LOOP_ITER 10
static int loop_iter;

ZTEST(posix_c_lib_ext, test_getopt)
{
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* This seems to be required by glibc in order to reliably reset getopt state */
		static const char *const argv[] = {"cmd", NULL};
		int argc = ARRAY_SIZE(argv) - 1;

		optind = 0;
		(void)getopt(argc, (GETOPT_ARGV_CAST)argv, "");
	}

	/* Test optind is incremented correctly */
	{
		static const char *const argv[] = {"cmd", "-a", "-b", "arg", "file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "ab:c");
		zexpect_equal('a', opt);
		zexpect_equal(2, optind, "optind should be 2 after parsing -a");

		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "ab:c");
		zexpect_equal('b', opt);
		zexpect_equal(4, optind, "optind should be 4 after parsing -b with argument");
		zexpect_str_equal("arg", optarg, "optarg should point to 'arg'");

		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "ab:c");
		zexpect_equal(-1, opt, "getopt should return -1 when no more options");
		zexpect_equal(4, optind, "optind should remain at 4");
	}

	/* Test that "--" terminates option processing */
	{
		static const char *const argv[] = {"cmd", "-a", "--", "-b", "file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "ab");
		zexpect_equal('a', opt);

		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "ab");
		zexpect_equal(-1, opt, "getopt should return -1 after '--'");
		zexpect_equal(3, optind, "optind should be incremented past '--'");
	}

	/* Test that a single "-" is not treated as an option */
	{
		static const char *const argv[] = {"cmd", "-", "file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "ab");
		zexpect_equal(-1, opt, "getopt should return -1 for single '-'");
		zexpect_equal(1, optind, "optind should not change for single '-'");
	}

	/* Test that optopt is set to the problematic option character */
	{
		static const char *const argv[] = {"cmd", "-z", "-a", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "ab:");
		zexpect_equal('?', opt, "getopt should return '?' for unknown option");
		zexpect_equal('z', optopt, "optopt should be set to 'z'");
	}

	/* Test optopt for missing argument */
	{
		static const char *const argv2[] = {"cmd", "-b", NULL};
		int argc2 = ARRAY_SIZE(argv2) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc2, (GETOPT_ARGV_CAST)argv2, "ab:");
		zexpect_equal('?', opt,
			      "getopt should return '?' for missing argument (no colon prefix)");
		zexpect_equal('b', optopt, "optopt should be set to 'b'");
	}

	/* Test that leading ':' in optstring changes error handling */
	{
		static const char *const argv[] = {"cmd", "-b", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, ":ab:");
		zexpect_equal(':', opt,
			      "getopt should return ':' for missing argument with colon prefix");
		zexpect_equal('b', optopt, "optopt should be set to 'b'");
	}

	/* Test unknown option with colon prefix */
	{
		static const char *const argv2[] = {"cmd", "-z", NULL};
		int argc2 = ARRAY_SIZE(argv2) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc2, (GETOPT_ARGV_CAST)argv2, ":ab:");
		zexpect_equal('?', opt,
			      "getopt should return '?' for unknown option even with colon prefix");
		zexpect_equal('z', optopt, "optopt should be set to 'z'");
	}

	/* Test option-argument immediately following option character */
	{
		static const char *const argv[] = {"cmd", "-ovalue", "-barg", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "o:b:");
		zexpect_equal('o', opt);
		zexpect_str_equal("value", optarg, "optarg should point to 'value'");
		zexpect_equal(2, optind, "optind should increment by 1 for adjacent argument");

		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "o:b:");
		zexpect_equal('b', opt);
		zexpect_str_equal("arg", optarg, "optarg should point to 'arg'");
		zexpect_equal(3, optind);
	}

	/* Test option-argument as next element */
	{
		static const char *const argv[] = {"cmd", "-o", "value", "-b", "arg", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "o:b:");
		zexpect_equal('o', opt);
		zexpect_str_equal("value", optarg, "optarg should point to 'value'");
		zexpect_equal(3, optind, "optind should increment by 2 for separated argument");

		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "o:b:");
		zexpect_equal('b', opt);
		zexpect_str_equal("arg", optarg, "optarg should point to 'arg'");
		zexpect_equal(5, optind);
	}

	/* Test multiple options in a single argv element (e.g., -abc) */
	{
		static const char *const argv[] = {"cmd", "-abc", "file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int a_count = 0, b_count = 0, c_count = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "abc")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			switch (opt) {
			case 'a':
				a_count++;
				break;
			case 'b':
				b_count++;
				break;
			case 'c':
				c_count++;
				break;
			}
		}

		zexpect_equal(1, a_count, "option 'a' should be parsed once");
		zexpect_equal(1, b_count, "option 'b' should be parsed once");
		zexpect_equal(1, c_count, "option 'c' should be parsed once");
		zexpect_equal(2, optind, "optind should point to first non-option argument");
	}

	/* Test combining options where the last one takes an argument */
	{
		static const char *const argv[] = {"cmd", "-abovalue", "file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int a_count = 0, b_count = 0;
		char *o_arg = NULL;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "abo:")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			switch (opt) {
			case 'a':
				a_count++;
				break;
			case 'b':
				b_count++;
				break;
			case 'o':
				o_arg = optarg;
				break;
			}
		}

		zexpect_equal(1, a_count);
		zexpect_equal(1, b_count);
		zexpect_not_null(o_arg);
		if (o_arg != NULL) {
			zexpect_str_equal("value", o_arg);
		}
	}

	/* Test that non-option arguments can be processed after option parsing
	 * (per Guideline 9, options must precede operands - once a non-option is
	 * encountered, option processing stops)
	 */
	{
		static const char *const argv[] = {"cmd",   "-a",    "file1", "-b",
						   "file2", "file3", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int option_count = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "ab")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			option_count++;
		}

		zexpect_equal(1, option_count, "should parse 1 option (before first operand)");

		/* After getopt returns -1, optind points to first non-option */
		int non_option_count = 0;

		loop_iter = 0;
		for (int i = optind; i < argc; i++) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			non_option_count++;
		}

		zexpect_equal(4, non_option_count, "remaining operands: file1, -b, file2, file3");
	}

	/* Test with empty optstring */
	{
		static const char *const argv[] = {"cmd", "-a", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "");
		zexpect_equal('?', opt, "any option should be unknown with empty optstring");
	}

	/* Test with no options in argv */
	{
		static const char *const argv[] = {"cmd", "file1", "file2", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "ab:");
		zexpect_equal(-1, opt, "getopt should return -1 when argv has no options");
		zexpect_equal(1, optind, "optind should remain at 1");
	}

	/* Test multiple options all requiring arguments */
	{
		static const char *const argv[] = {"cmd",  "-a", "arg1", "-b",
						   "arg2", "-c", "arg3", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		char *a_arg = NULL, *b_arg = NULL, *c_arg = NULL;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "a:b:c:")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			switch (opt) {
			case 'a':
				a_arg = optarg;
				break;
			case 'b':
				b_arg = optarg;
				break;
			case 'c':
				c_arg = optarg;
				break;
			}
		}

		zexpect_str_equal("arg1", a_arg);
		zexpect_str_equal("arg2", b_arg);
		zexpect_str_equal("arg3", c_arg);
	}

	/* Test option as last element of argv */
	{
		static const char *const argv[] = {"cmd", "file", "-a", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "a");
		zexpect_equal(-1, opt, "option after non-option should not be parsed");
	}

	/* Test option requiring argument as last element */
	{
		static const char *const argv[] = {"cmd", "-o", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "o:");
		zexpect_equal('?', opt, "missing argument should return '?'");
		zexpect_equal('o', optopt);
	}

	/* Test that "-" can be used as an option argument */
	{
		static const char *const argv[] = {"cmd", "-o", "-", "file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "o:");
		zexpect_equal('o', opt);
		zexpect_str_equal("-", optarg, "'-' should be accepted as argument");
	}

	/* Test the same option appearing multiple times */
	{
		static const char *const argv[] = {"cmd", "-a", "-a", "-a", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int count = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "a")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			if (opt == 'a') {
				count++;
			}
		}

		zexpect_equal(3, count, "option 'a' should be counted 3 times");
	}

	/* Test numeric option characters (allowed by spec as extension) */
	{
		static const char *const argv[] = {"cmd", "-1", "-2", "-9", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int found_1 = 0, found_2 = 0, found_9 = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "123456789")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			switch (opt) {
			case '1':
				found_1 = 1;
				break;
			case '2':
				found_2 = 1;
				break;
			case '9':
				found_9 = 1;
				break;
			}
		}

		zexpect_equal(1, found_1);
		zexpect_equal(1, found_2);
		zexpect_equal(1, found_9);
	}

	/* Guideline 5: One or more options without option-arguments, followed by
	 * at most one option that takes an option-argument, should be accepted when
	 * grouped behind one '-' delimiter. (multiple options without args + one with arg -
	 * adjacent)
	 */
	{
		static const char *const argv[] = {"cmd", "-abcovalue", "file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int a_count = 0, b_count = 0, c_count = 0;
		char *o_arg = NULL;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "abco:")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			switch (opt) {
			case 'a':
				a_count++;
				break;
			case 'b':
				b_count++;
				break;
			case 'c':
				c_count++;
				break;
			case 'o':
				o_arg = optarg;
				break;
			}
		}

		zexpect_equal(1, a_count, "option 'a' parsed once");
		zexpect_equal(1, b_count, "option 'b' parsed once");
		zexpect_equal(1, c_count, "option 'c' parsed once");
		zexpect_not_null(o_arg, "option 'o' should have argument");
		if (o_arg != NULL) {
			zexpect_str_equal("value", o_arg);
		}
	}

	/* Guideline 5: multiple options without args, followed by one with arg (separated) */
	{
		static const char *const argv[] = {"cmd", "-abc", "-o", "value", "file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int a_count = 0, b_count = 0, c_count = 0;
		char *o_arg = NULL;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "abco:")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			switch (opt) {
			case 'a':
				a_count++;
				break;
			case 'b':
				b_count++;
				break;
			case 'c':
				c_count++;
				break;
			case 'o':
				o_arg = optarg;
				break;
			}
		}

		zexpect_equal(1, a_count);
		zexpect_equal(1, b_count);
		zexpect_equal(1, c_count);
		zexpect_not_null(o_arg);
		if (o_arg != NULL) {
			zexpect_str_equal("value", o_arg);
		}
	}

	/* Guideline 5: single option with argument in group */
	{
		static const char *const argv[] = {"cmd", "-ovalue", "file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "o:");
		zexpect_equal('o', opt);
		zexpect_str_equal("value", optarg);
	}

	/* Guideline 6: mandatory option-argument as separate argument */
	{
		static const char *const argv[] = {"cmd", "-f", "filename", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "f:");
		zexpect_equal('f', opt);
		zexpect_str_equal("filename", optarg);
		zexpect_equal(3, optind, "optind incremented by 2 for separated arg");
	}

	/* Guideline 6: mandatory option-argument adjacent (also allowed) */
	{
		static const char *const argv[] = {"cmd", "-ffilename", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "f:");
		zexpect_equal('f', opt);
		zexpect_str_equal("filename", optarg);
		zexpect_equal(2, optind, "optind incremented by 1 for adjacent arg");
	}

	/* Guideline 7: option requiring argument appears at end of argv (error) */
	{
		static const char *const argv[] = {"cmd", "file", "-f", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "f:");
		zexpect_equal(-1, opt, "option after operand should not be parsed");
	}

	/* Guideline 7: option requiring argument but next arg is another option */
	{
		static const char *const argv[] = {"cmd", "-f", "-a", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "f:a");
		zexpect_equal('f', opt);
		zexpect_str_equal(
			"-a", optarg,
			"next argument taken as option-argument even if it looks like option");
	}

	/* Guideline 7: option requiring argument gets "--" as argument */
	{
		static const char *const argv[] = {"cmd", "-f", "--", "file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "f:");
		zexpect_equal('f', opt);
		zexpect_str_equal("--", optarg, "'--' should be taken as option-argument");

		/* After consuming '--' as argument, parsing continues */
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "f:");
		zexpect_equal(-1, opt);
	}

	/* Guideline 9: option after operand is not processed */
	{
		static const char *const argv[] = {"cmd", "operand", "-a", "-b", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int option_count = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "ab")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			option_count++;
		}

		zexpect_equal(0, option_count, "no options should be parsed after operand");
		zexpect_equal(1, optind, "optind should stop at first operand");
	}

	/* Guideline 9: all options before operands are processed */
	{
		static const char *const argv[] = {"cmd", "-a", "-b", "op1", "op2", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int option_count = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "ab")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			option_count++;
		}

		zexpect_equal(2, option_count, "both options before operands processed");
		zexpect_equal(3, optind, "optind points to first operand");
	}

	/* Guideline 10: '--' terminates option processing, following '-x' treated as operand */
	{
		static const char *const argv[] = {"cmd", "-a", "--", "-b", "-c", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int a_count = 0, b_count = 0, c_count = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "abc")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			switch (opt) {
			case 'a':
				a_count++;
				break;
			case 'b':
				b_count++;
				break;
			case 'c':
				c_count++;
				break;
			}
		}

		zexpect_equal(1, a_count, "option 'a' before '--' is processed");
		zexpect_equal(0, b_count, "option 'b' after '--' is not processed");
		zexpect_equal(0, c_count, "option 'c' after '--' is not processed");
		zexpect_equal(3, optind, "optind points past '--'");

		/* Remaining arguments should be available as operands */
		zexpect_equal(argc - optind, 2, "two operands remain");
		zexpect_str_equal(argv[optind], "-b", "first operand is '-b'");
		zexpect_str_equal(argv[optind + 1], "-c", "second operand is '-c'");
	}

	/* Guideline 10: '--' with no options before it */
	{
		static const char *const argv[] = {"cmd", "--", "operand", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "abc");
		zexpect_equal(-1, opt);
		zexpect_equal(2, optind, "optind points past '--'");
	}

	/* Guideline 10: multiple '--' arguments (only first is delimiter) */
	{
		static const char *const argv[] = {"cmd", "-a", "--", "--", "operand", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "a");
		zexpect_equal('a', opt);

		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "a");
		zexpect_equal(-1, opt);
		zexpect_equal(3, optind, "optind points past first '--'");

		/* Second '--' is now an operand */
		zexpect_str_equal(argv[optind], "--", "second '--' is an operand");
	}

	/* Guideline 3: single letter options work */
	{
		static const char *const argv[] = {"cmd", "-a", "-z", "-A", "-Z", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int count = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "azAZ")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			zexpect_true(opt == 'a' || opt == 'z' || opt == 'A' || opt == 'Z',
				     "option should be single character");
			count++;
		}

		zexpect_equal(4, count, "all single character options parsed");
	}

	/* Guideline 3: single digit options work */
	{
		static const char *const argv[] = {"cmd", "-0", "-5", "-9", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int count = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "0123456789")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			zexpect_true(opt >= '0' && opt <= '9', "option should be digit");
			count++;
		}

		zexpect_equal(3, count, "all digit options parsed");
	}

	/* Guideline 4: options must start with '-' */
	{
		static const char *const argv[] = {"cmd", "abc", "-d", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "abcd");
		zexpect_equal(-1, opt, "'abc' without '-' prefix not treated as options");
	}

	/* Guideline 4: '-' prefix is required */
	{
		static const char *const argv[] = {"cmd", "-a", "b", "-c", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int a_count = 0, b_count = 0, c_count = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "abc")) != -1) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			switch (opt) {
			case 'a':
				a_count++;
				break;
			case 'b':
				b_count++;
				break;
			case 'c':
				c_count++;
				break;
			}
		}

		zexpect_equal(1, a_count, "'-a' is processed");
		zexpect_equal(0, b_count, "'b' without '-' is not processed as option");
		zexpect_equal(0, c_count, "'-c' after operand is not processed");
	}
}

ZTEST(posix_c_lib_ext, test_getopt_spec_example)
{
	/* clang-format off */
	static const char *const test_argv[][8] = {
		{"cmd", "-ao", "arg", "path", "path"},
		{"cmd", "-a", "-o", "arg", "path", "path"},
		{"cmd", "-o", "arg", "-a", "path", "path"},
		{"cmd", "-a", "-o", "arg", "--", "path", "path"},
		{"cmd", "-a", "-oarg", "path", "path"},
		{"cmd", "-aoarg", "path", "path"},
	};
	/* clang-format on */
	static const int test_argc[] = {5, 6, 6, 7, 5, 4};

	ARRAY_FOR_EACH(test_argv, i) {
		int argc = test_argc[i];
		char **argv = (char **)test_argv[i];
		static const char *optstring = ":abf:o:";

		int c = 0;
		int bflg = 0, aflg = 0, errflg = 0;
		char *ifile = NULL;
		char *ofile = NULL;

		optind = 1;
		loop_iter = 0;
		while (true) {
			if (loop_iter > TEST_MAX_LOOP_ITER) {
				break;
			}
			loop_iter++;

			c = getopt(argc, (GETOPT_ARGV_CAST)argv, optstring);
			if (c == -1) {
				break;
			}

			switch (c) {
			case 'a':
				if (bflg) {
					errflg++;
				} else {
					aflg++;
				}
				break;
			case 'b':
				if (aflg) {
					errflg++;
				} else {
					bflg++;
				}
				break;
			case 'f':
				ifile = optarg;
				break;
			case 'o':
				ofile = optarg;
				break;
			case ':': /* -f or -o without operand */
				fprintf(stderr, "Option -%c requires an operand\n", optopt);
				errflg++;
				break;
			case '?':
				fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
				errflg++;
				break;
			}
		}

		zexpect_equal(0, errflg, "[case %zu] %d errors detected", i, errflg);
		zexpect_equal(0, bflg, "[case %zu] expected bflg to be 0 but was %d", i, bflg);
		zexpect_is_null(ifile, "[case %zu]", i);
		zexpect_not_null(ofile, "[case %zu]", i);
		if (ofile != NULL) {
			zexpect_str_equal("arg", ofile, "[case %zu]", i);
		}
	}
}

/* Note4:
 * Another corner case that produces non-ideal results is
 *
 * getopt(5, ["cmd", "-a", "arg", "-b"], "a::\nb")
 *
 * Ideally getopt() would ignore characters in optstring that do not correspond to [:alnum:] digits.
 * The glibc implementation returns 'a' on the first call, without setting optarg, and then returns
 * -1 on the second call.
 */

ZTEST(posix_c_lib_ext, test_getopt_repeated_opts)
{
	/* Tests that getopt() allows optstring to contain the same option multiple times */
	{
		static const char *const argv[] = {"cmd", "-a", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "aa");
		zexpect_equal('a', opt, "expected 'a' option. actual: '%c' (%d)",
			      isprint(opt) ? opt : '.', opt);
	}
}

ZTEST(posix_c_lib_ext, test_getopt_repeated_opts_conflicting_args)
{
	/* Tests that getopt() does not allow specifying an argument for a previously specified
	 * option that did not have one (e.g. opstring = "aa:")
	 */
	{
		static const char *const argv[] = {"cmd", "-a", "-a", "arg", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		optarg = NULL;
		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "aa:");
		zexpect_equal('a', opt, "expected 'a' option. actual: '%c' (%d)",
			      isprint(opt) ? opt : '.', opt);
		zexpect_is_null(optarg, "expected optarg to be NULL");

		opt = getopt(argc, (GETOPT_ARGV_CAST)argv, "aa:");
		zexpect_equal('a', opt, "expected 'a' option. actual: '%c' (%d)",
			      isprint(opt) ? opt : '.', opt);
		zexpect_is_null(optarg, "expected optarg to be NULL");
	}
}
