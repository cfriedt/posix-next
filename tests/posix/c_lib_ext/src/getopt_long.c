/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ctype.h>
#include <getopt.h>
#include <stdio.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#if defined(CONFIG_NATIVE_LIBC)
/* glibc() use char *argv[], which breaks conformance */
#define GETOPT_ARGV_CAST char **
#else
/* POSIX requires the argv array to be of type char *const [] */
#define GETOPT_ARGV_CAST char *const *
#endif

#define TEST_MAX_LOOP_ITER 10
static int loop_iter;

ZTEST(getopt_long, test_getopt_long)
{
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* This seems to be required by glibc in order to reliably reset getopt state */
		static const char *const argv[] = {"cmd", NULL};
		int argc = ARRAY_SIZE(argv) - 1;

		optind = 0;
		(void)getopt(argc, (GETOPT_ARGV_CAST)argv, "");
	}

	/* Test basic long option without argument */
	{
		static const struct option long_options[] = {
			{"verbose", no_argument, NULL, 'v'},
			{"help", no_argument, NULL, 'h'},
			{0, 0, 0, 0},
		};
		static const char *const argv[] = {"cmd", "--verbose", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, "vh", long_options, NULL);
		zexpect_equal('v', opt, "getopt_long should return 'v' for --verbose");
	}

	/* Test long option with required argument (separate) */
	{
		static const struct option long_options[] = {
			{"file", required_argument, NULL, 'f'},
			{0, 0, 0, 0},
		};
		static const char *const argv[] = {"cmd", "--file", "myfile.txt", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, "f:", long_options, NULL);
		zexpect_equal('f', opt);
		zexpect_str_equal("myfile.txt", optarg, "optarg should be 'myfile.txt'");
	}

	/* Test long option with required argument using = syntax */
	{
		static const struct option long_options[] = {
			{"file", required_argument, NULL, 'f'},
			{0, 0, 0, 0},
		};
		static const char *const argv[] = {"cmd", "--file=myfile.txt", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, "f:", long_options, NULL);
		zexpect_equal('f', opt);
		zexpect_str_equal("myfile.txt", optarg, "optarg should be 'myfile.txt'");
	}

	/* Test multiple long options in sequence */
	{
		static const struct option long_options[] = {
			{"verbose", no_argument, NULL, 'v'},
			{"file", required_argument, NULL, 'f'},
			{0, 0, 0, 0},
		};
		static const char *const argv[] = {"cmd", "--verbose", "--file", "test.txt", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int v_count = 0;
		char *f_arg = NULL;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, "vf:", long_options,
					  NULL)) != -1) {
			zassert_true(loop_iter < TEST_MAX_LOOP_ITER);
			loop_iter++;

			switch (opt) {
			case 'v':
				v_count++;
				break;
			case 'f':
				f_arg = optarg;
				break;
			}
		}

		zexpect_equal(1, v_count);
		zexpect_not_null(f_arg);
		if (f_arg != NULL) {
			zexpect_str_equal("test.txt", f_arg);
		}
	}

	/* Test longindex parameter returns correct index */
	{
		static const struct option long_options[] = {
			{"verbose", no_argument, NULL, 'v'},
			{"file", required_argument, NULL, 'f'},
			{"output", required_argument, NULL, 'o'},
			{0, 0, 0, 0},
		};
		static const char *const argv[] = {"cmd", "--output", "out.txt", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int option_index = -1;

		optind = 1;
		opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, "vf:o:", long_options,
				  &option_index);
		zexpect_equal('o', opt);
		zexpect_equal(2, option_index, "option_index should be 2 for 'output' option");
		zexpect_str_equal("out.txt", optarg);
	}

	/* Test flag behavior: flag != NULL, getopt_long returns 0 and sets *flag to val */
	{
		static int verbose_flag;
		static int debug_flag;
		static const struct option long_options[] = {
			{"verbose", no_argument, &verbose_flag, 1},
			{"debug", no_argument, &debug_flag, 1},
			{0, 0, 0, 0},
		};
		static const char *const argv[] = {"cmd", "--verbose", "--debug", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		verbose_flag = 0;
		debug_flag = 0;
		optind = 1;
		loop_iter = 0;
		while ((opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, "", long_options, NULL)) !=
		       -1) {
			zassert_true(loop_iter < TEST_MAX_LOOP_ITER);
			loop_iter++;

			/* When flag is not NULL, getopt_long returns 0 */
			zexpect_equal(0, opt, "getopt_long should return 0 when flag is not NULL");
		}

		zexpect_equal(1, verbose_flag, "verbose_flag should be set to 1");
		zexpect_equal(1, debug_flag, "debug_flag should be set to 1");
	}

	/* Test flag with different values */
	{
		static int mode_flag;
		static const struct option long_options[] = {
			{"fast", no_argument, &mode_flag, 1},
			{"slow", no_argument, &mode_flag, 2},
			{0, 0, 0, 0},
		};
		static const char *const argv[] = {"cmd", "--fast", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		mode_flag = 0;
		optind = 1;
		opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, "", long_options, NULL);
		zexpect_equal(0, opt);
		zexpect_equal(1, mode_flag, "mode_flag should be 1 for --fast");
	}

	/* Test mixing short and long options */
	{
		static const struct option long_options[] = {
			{"verbose", no_argument, NULL, 'v'},
			{"file", required_argument, NULL, 'f'},
			{0, 0, 0, 0},
		};
		static const char *const argv[] = {"cmd", "-v", "--file", "test.txt", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int v_count = 0;
		char *f_arg = NULL;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, "vf:", long_options,
					  NULL)) != -1) {
			zassert_true(loop_iter < TEST_MAX_LOOP_ITER);
			loop_iter++;

			switch (opt) {
			case 'v':
				v_count++;
				break;
			case 'f':
				f_arg = optarg;
				break;
			}
		}

		zexpect_equal(1, v_count, "short option -v recognized");
		zexpect_not_null(f_arg);
		if (f_arg != NULL) {
			zexpect_str_equal("test.txt", f_arg);
		}
	}

	/* Test long option with optional argument (value provided) */
	{
		static const struct option long_options[] = {
			{"config", optional_argument, NULL, 'c'},
			{0, 0, 0, 0},
		};
		static const char *const argv[] = {"cmd", "--config=myconfig.txt", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, "c::", long_options, NULL);
		zexpect_equal('c', opt);
		zexpect_str_equal("myconfig.txt", optarg);
	}

	/* Test unknown long option returns '?' */
	{
		static const struct option long_options[] = {
			{"verbose", no_argument, NULL, 'v'},
			{0, 0, 0, 0},
		};
		static const char *const argv[] = {"cmd", "--unknown", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, "v", long_options, NULL);
		zexpect_equal('?', opt, "unknown option should return '?'");
	}

	/* Test missing required argument returns '?' */
	{
		static const struct option long_options[] = {
			{"file", required_argument, NULL, 'f'},
			{0, 0, 0, 0},
		};
		static const char *const argv[] = {"cmd", "--file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, "f:", long_options, NULL);
		zexpect_equal('?', opt, "missing argument should return '?'");
	}

	/* Test colon prefix for different error reporting */
	{
		static const struct option long_options[] = {
			{"file", required_argument, NULL, 'f'},
			{0, 0, 0, 0},
		};
		static const char *const argv[] = {"cmd", "--file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, ":f:", long_options, NULL);
		zexpect_equal(':', opt, "with ':' prefix should return ':' for missing argument");
	}

	/* Test '--' terminates option processing */
	{
		static const struct option long_options[] = {
			{"verbose", no_argument, NULL, 'v'},
			{0, 0, 0, 0},
		};
		static const char *const argv[] = {"cmd", "-v", "--", "--verbose", "file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int v_count = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, "v", long_options, NULL)) !=
		       -1) {
			zassert_true(loop_iter < TEST_MAX_LOOP_ITER);
			loop_iter++;

			if (opt == 'v') {
				v_count++;
			}
		}

		zexpect_equal(1, v_count, "only one -v before '--' processed");
		zexpect_equal(3, optind, "optind should point past '--'");
		zexpect_str_equal("--verbose", argv[optind], "after '--' should be operand");
	}
}

ZTEST(getopt_long, test_getopt_long_example)
{
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* This seems to be required by glibc in order to reliably reset getopt state */
		static const char *const argv[] = {"cmd", NULL};
		int argc = ARRAY_SIZE(argv) - 1;

		optind = 0;
		(void)getopt(argc, (GETOPT_ARGV_CAST)argv, "");
	}

	/* Test example from man page */
	{
		static const struct option long_options[] = {
			{"add", required_argument, NULL, 'a'},
			{"append", no_argument, NULL, 'A'},
			{"delete", required_argument, NULL, 'd'},
			{"verbose", no_argument, NULL, 'v'},
			{"create", required_argument, NULL, 'c'},
			{"file", required_argument, NULL, 'f'},
			{0, 0, 0, 0}};
		static const char *const argv[] = {"cmd",    "--verbose", "--add", "item1",
						   "--file", "data.txt",  NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int verbose_count = 0;
		char *add_arg = NULL;
		char *file_arg = NULL;
		int option_index = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt_long(argc, (GETOPT_ARGV_CAST)argv, "a:Ad:vc:f:", long_options,
					  &option_index)) != -1) {
			zassert_true(loop_iter < TEST_MAX_LOOP_ITER);
			loop_iter++;

			switch (opt) {
			case 'a':
				add_arg = optarg;
				break;
			case 'v':
				verbose_count++;
				break;
			case 'f':
				file_arg = optarg;
				break;
			}
		}

		zexpect_equal(1, verbose_count);
		zexpect_not_null(add_arg);
		if (add_arg != NULL) {
			zexpect_str_equal("item1", add_arg);
		}
		zexpect_not_null(file_arg);
		if (file_arg != NULL) {
			zexpect_str_equal("data.txt", file_arg);
		}
	}
}

ZTEST(getopt_long, test_getopt_long_only)
{
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* This seems to be required by glibc in order to reliably reset getopt state */
		static const char *const argv[] = {"cmd", NULL};
		int argc = ARRAY_SIZE(argv) - 1;

		optind = 0;
		(void)getopt(argc, (GETOPT_ARGV_CAST)argv, "");
	}

	/* Test single dash long option */
	{
		static const struct option long_options[] = {{"verbose", no_argument, NULL, 'v'},
							     {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-verbose", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "v", long_options, NULL);
		zexpect_equal('v', opt, "getopt_long_only should match -verbose as long option");
	}

	/* Test double dash still works */
	{
		static const struct option long_options[] = {{"verbose", no_argument, NULL, 'v'},
							     {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "--verbose", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "v", long_options, NULL);
		zexpect_equal('v', opt, "getopt_long_only should support --verbose");
	}

	/* Test single dash with required argument (separate) */
	{
		static const struct option long_options[] = {{"file", required_argument, NULL, 'f'},
							     {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-file", "test.txt", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "f:", long_options, NULL);
		zexpect_equal('f', opt);
		zexpect_str_equal("test.txt", optarg);
	}

	/* Test single dash with required argument (= syntax) */
	{
		static const struct option long_options[] = {{"file", required_argument, NULL, 'f'},
							     {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-file=test.txt", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "f:", long_options, NULL);
		zexpect_equal('f', opt);
		zexpect_str_equal("test.txt", optarg);
	}

	/* Test single dash non-matching falls back to short option */
	{
		static const struct option long_options[] = {{"verbose", no_argument, NULL, 'v'},
							     {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-a", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "av", long_options, NULL);
		zexpect_equal('a', opt,
			      "non-matching single dash should fall back to short option");
	}

	/* Test mixing single and double dash long options */
	{
		static const struct option long_options[] = {{"verbose", no_argument, NULL, 'v'},
							     {"file", required_argument, NULL, 'f'},
							     {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-verbose", "--file", "test.txt", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int v_count = 0;
		char *f_arg = NULL;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "vf:", long_options,
					       NULL)) != -1) {
			zassert_true(loop_iter < TEST_MAX_LOOP_ITER);
			loop_iter++;

			switch (opt) {
			case 'v':
				v_count++;
				break;
			case 'f':
				f_arg = optarg;
				break;
			}
		}

		zexpect_equal(1, v_count);
		zexpect_not_null(f_arg);
		if (f_arg != NULL) {
			zexpect_str_equal("test.txt", f_arg);
		}
	}

	/* Test short option still works */
	{
		static const struct option long_options[] = {{"verbose", no_argument, NULL, 'v'},
							     {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-v", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "v", long_options, NULL);
		zexpect_equal('v', opt, "short option -v should still work");
	}

	/* Test longindex parameter with single dash */
	{
		static const struct option long_options[] = {
			{"verbose", no_argument, NULL, 'v'},
			{"file", required_argument, NULL, 'f'},
			{"output", required_argument, NULL, 'o'},
			{0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-output", "out.txt", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int option_index = -1;

		optind = 1;
		opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "vf:o:", long_options,
				       &option_index);
		zexpect_equal('o', opt);
		zexpect_equal(2, option_index, "option_index should be 2 for 'output'");
		zexpect_str_equal("out.txt", optarg);
	}

	/* Test flag behavior with single dash */
	{
		static int verbose_flag;
		static const struct option long_options[] = {
			{"verbose", no_argument, &verbose_flag, 1}, {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-verbose", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		verbose_flag = 0;
		optind = 1;
		opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "", long_options, NULL);
		zexpect_equal(0, opt, "getopt_long_only should return 0 when flag is set");
		zexpect_equal(1, verbose_flag, "verbose_flag should be set to 1");
	}

	/* Test optional argument with single dash */
	{
		static const struct option long_options[] = {
			{"config", optional_argument, NULL, 'c'}, {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-config=myconfig.txt", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "c::", long_options, NULL);
		zexpect_equal('c', opt);
		zexpect_str_equal("myconfig.txt", optarg);
	}

	/* Test unknown option returns '?' */
	{
		static const struct option long_options[] = {{"verbose", no_argument, NULL, 'v'},
							     {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-unknown", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "v", long_options, NULL);
		zexpect_equal('?', opt, "unknown option should return '?'");
	}

	/* Test missing required argument */
	{
		static const struct option long_options[] = {{"file", required_argument, NULL, 'f'},
							     {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "f:", long_options, NULL);
		zexpect_equal('?', opt, "missing argument should return '?'");
	}

	/* Test colon prefix for different error reporting */
	{
		static const struct option long_options[] = {{"file", required_argument, NULL, 'f'},
							     {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-file", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;

		optind = 1;
		opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, ":f:", long_options, NULL);
		zexpect_equal(':', opt, "with ':' prefix should return ':' for missing argument");
	}

	/* Test '--' terminates option processing */
	{
		static const struct option long_options[] = {{"verbose", no_argument, NULL, 'v'},
							     {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd",      "-verbose", "--",
						   "-verbose", "file",     NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int v_count = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "v", long_options,
					       NULL)) != -1) {
			zassert_true(loop_iter < TEST_MAX_LOOP_ITER);
			loop_iter++;

			if (opt == 'v') {
				v_count++;
			}
		}

		zexpect_equal(1, v_count, "only one -verbose before '--' processed");
		zexpect_equal(3, optind, "optind should point past '--'");
		zexpect_str_equal("-verbose", argv[optind], "after '--' should be operand");
	}

	/* Test multiple options in sequence */
	{
		static const struct option long_options[] = {{"verbose", no_argument, NULL, 'v'},
							     {"file", required_argument, NULL, 'f'},
							     {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-verbose", "-file", "data.txt", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int v_count = 0;
		char *f_arg = NULL;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "vf:", long_options,
					       NULL)) != -1) {
			zassert_true(loop_iter < TEST_MAX_LOOP_ITER);
			loop_iter++;

			switch (opt) {
			case 'v':
				v_count++;
				break;
			case 'f':
				f_arg = optarg;
				break;
			}
		}

		zexpect_equal(1, v_count);
		zexpect_not_null(f_arg);
		if (f_arg != NULL) {
			zexpect_str_equal("data.txt", f_arg);
		}
	}

	/* Test grouped short options with long options */
	{
		static const struct option long_options[] = {{"verbose", no_argument, NULL, 'v'},
							     {0, 0, 0, 0}};
		static const char *const argv[] = {"cmd", "-ab", "-verbose", NULL};
		int argc = ARRAY_SIZE(argv) - 1;
		int opt;
		int a_count = 0, b_count = 0, v_count = 0;

		optind = 1;
		loop_iter = 0;
		while ((opt = getopt_long_only(argc, (GETOPT_ARGV_CAST)argv, "abv", long_options,
					       NULL)) != -1) {
			zassert_true(loop_iter < TEST_MAX_LOOP_ITER);
			loop_iter++;

			switch (opt) {
			case 'a':
				a_count++;
				break;
			case 'b':
				b_count++;
				break;
			case 'v':
				v_count++;
				break;
			}
		}

		zexpect_equal(1, a_count);
		zexpect_equal(1, b_count);
		zexpect_equal(1, v_count);
	}
}

ZTEST_SUITE(getopt_long, NULL, NULL, NULL, NULL, NULL);
