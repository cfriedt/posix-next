/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <locale.h>
#include <monetary.h>
#include <string.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#define HRULE "------------------------------------------------"

struct test_data {
	int tag;
	char *s;
	size_t maxsize;
	const char *format;
	bool expected_success;
	int expected_errno;
	const char *expected_buffer[3];
};

enum strfmon_tc_enum {
	STRFMON_TC_ESCAPE_PERCENT,
	STRFMON_TC_DEFAULT,
	STRFMON_TC_RIGHT_ALIGN,
	STRFMON_TC_ALIGNED_COLUMNS,
	STRFMON_TC_FILL_CHAR,
	STRFMON_TC_NO_FILL_GROUPING,
	STRFMON_TC_DISABLE_GROUPING,
	STRFMON_TC_ROUND_WHOLE,
	STRFMON_TC_INCREASE_PRECISION,
	STRFMON_TC_ALTERNATE_POS_NEG,
	STRFMON_TC_NO_CURRENCY_SYMBOL,
	STRFMON_TC_LEFT_JUSTIFY,
	STRFMON_TC_RIGHT_JUSTIFY,
};

static const char *const strfmon_tc_desc[] = {
	[STRFMON_TC_ESCAPE_PERCENT] = "Escape percent sign",
	[STRFMON_TC_DEFAULT] = "Default formatting",
	[STRFMON_TC_RIGHT_ALIGN] = "Right align within an 11-character field",
	[STRFMON_TC_ALIGNED_COLUMNS] = "Aligned columns for values up to 99999",
	[STRFMON_TC_FILL_CHAR] = "Specify a fill character",
	[STRFMON_TC_NO_FILL_GROUPING] = "Fill characters do not use grouping",
	[STRFMON_TC_DISABLE_GROUPING] = "Disable the grouping separator",
	[STRFMON_TC_ROUND_WHOLE] = "Round off to whole units",
	[STRFMON_TC_INCREASE_PRECISION] = "Increase the precision",
	[STRFMON_TC_ALTERNATE_POS_NEG] = "Use an alternative pos/neg style",
	[STRFMON_TC_NO_CURRENCY_SYMBOL] = "Disable the currency symbol",
	[STRFMON_TC_LEFT_JUSTIFY] = "Left-justify the output",
	[STRFMON_TC_RIGHT_JUSTIFY] = "Corresponding right-justified output",
};

static const char *const strfmon_tc_fmt[] = {
	/* clang-format off */
	[STRFMON_TC_ESCAPE_PERCENT] = "%%",
	[STRFMON_TC_DEFAULT] = "%n",
	[STRFMON_TC_RIGHT_ALIGN] = "%11n",
	[STRFMON_TC_ALIGNED_COLUMNS] = "%#5n",
	[STRFMON_TC_FILL_CHAR] = "%=*#5n",
	[STRFMON_TC_NO_FILL_GROUPING] = "%=0#5n",
	[STRFMON_TC_DISABLE_GROUPING] = "%^#5n",
	[STRFMON_TC_ROUND_WHOLE] = "%^#5.0n",
	[STRFMON_TC_INCREASE_PRECISION] = "%^#5.4n",
	[STRFMON_TC_ALTERNATE_POS_NEG] = "%(#5n",
	[STRFMON_TC_NO_CURRENCY_SYMBOL] = "%!(#5n",
	[STRFMON_TC_LEFT_JUSTIFY] = "%-14#5.4n",
	[STRFMON_TC_RIGHT_JUSTIFY] = "%14#5.4n",
	/* clang-format on */
};

#define STRFMON_TC_ERR_DECL(_buf, _bufsize, _fmt, _err)                                            \
	{                                                                                          \
		.tag = -1, .s = _buf, .maxsize = _bufsize, .format = _fmt,                         \
		.expected_success = false, .expected_errno = _err,                                 \
		.expected_buffer = {                                                               \
			NULL,                                                                      \
			NULL,                                                                      \
			NULL,                                                                      \
		},                                                                                 \
	}

#define STRFMON_TC_OK_DECL(_tag, a, b, c)                                                          \
	{                                                                                          \
		.tag = STRFMON_TC_##_tag, .s = buffer, .maxsize = sizeof(buffer),                  \
		.format = strfmon_tc_fmt[STRFMON_TC_##_tag], .expected_success = true,             \
		.expected_errno = 0,                                                               \
		.expected_buffer = {                                                               \
			a,                                                                         \
			b,                                                                         \
			c,                                                                         \
		},                                                                                 \
	}

static char buffer[15];
static const double input[] = {
	123.45,
	-123.45,
	3456.781,
};

static const struct test_data data[] = {
	/* expected failures - commented lines will cause e.g. glibc to segfault */

	/* STRFMON_TC_ERR_DECL(NULL, 0, NULL, EINVAL), */
	STRFMON_TC_ERR_DECL(NULL, 0, "", E2BIG),
	/* STRFMON_TC_ERR_DECL(NULL, sizeof(buffer), NULL, EINVAL), */
	/* STRFMON_TC_ERR_DECL(NULL, sizeof(buffer), "", EINVAL), */
	/* STRFMON_TC_ERR_DECL(buffer, 0, NULL, EINVAL), */
	STRFMON_TC_ERR_DECL(buffer, 0, "", E2BIG),
	/* STRFMON_TC_ERR_DECL(buffer, sizeof(buffer), NULL, EINVAL), */

	STRFMON_TC_ERR_DECL(buffer, 0, "%n", E2BIG),
	STRFMON_TC_ERR_DECL(buffer, sizeof(buffer), "%", EINVAL),

	/* happy path */
	STRFMON_TC_OK_DECL(ESCAPE_PERCENT, "%", "%", "%"),

#if defined(CONFIG_TEST_LOCALE_CHOICE_C)
	STRFMON_TC_OK_DECL(DEFAULT, "123.45", "-123.45", "3456.78"),
	STRFMON_TC_OK_DECL(RIGHT_ALIGN, "     123.45", "    -123.45", "    3456.78"),
	STRFMON_TC_OK_DECL(ALIGNED_COLUMNS, "   123.45", "-  123.45", "  3456.78"),
	STRFMON_TC_OK_DECL(FILL_CHAR, " **123.45", "-**123.45", " *3456.78"),
	STRFMON_TC_OK_DECL(NO_FILL_GROUPING, " 00123.45", "-00123.45", " 03456.78"),
	STRFMON_TC_OK_DECL(DISABLE_GROUPING, "   123.45", "-  123.45", "  3456.78"),
	STRFMON_TC_OK_DECL(ROUND_WHOLE, "   123", "-  123", "  3457"),
	STRFMON_TC_OK_DECL(INCREASE_PRECISION, "   123.4500", "-  123.4500", "  3456.7810"),
	STRFMON_TC_OK_DECL(ALTERNATE_POS_NEG, "   123.45", "(  123.45)", "  3456.78"),
	STRFMON_TC_OK_DECL(NO_CURRENCY_SYMBOL, "   123.45", "(  123.45)", "  3456.78"),
	STRFMON_TC_OK_DECL(LEFT_JUSTIFY, "   123.4500   ", "-  123.4500   ", "  3456.7810   "),
	STRFMON_TC_OK_DECL(RIGHT_JUSTIFY, "      123.4500", "   -  123.4500", "     3456.7810"),
#elif defined(CONFIG_TEST_LOCALE_CHOICE_EN_US_UTF8)
	STRFMON_TC_OK_DECL(DEFAULT, "$123.45", "-$123.45", "$3,456.78"),
	STRFMON_TC_OK_DECL(RIGHT_ALIGN, "    $123.45", "   -$123.45", "  $3,456.78"),
	STRFMON_TC_OK_DECL(ALIGNED_COLUMNS, " $   123.45", "-$   123.45", " $ 3,456.78"),
	STRFMON_TC_OK_DECL(FILL_CHAR, " $***123.45", "-$***123.45", " $*3,456.78"),
	STRFMON_TC_OK_DECL(NO_FILL_GROUPING, " $000123.45", "-$000123.45", " $03,456.78"),
	STRFMON_TC_OK_DECL(DISABLE_GROUPING, " $  123.45", "-$  123.45", " $ 3456.78"),
	STRFMON_TC_OK_DECL(ROUND_WHOLE, " $  123", "-$  123", " $ 3457"),
	STRFMON_TC_OK_DECL(INCREASE_PRECISION, " $  123.4500", "-$  123.4500", " $ 3456.7810"),
	STRFMON_TC_OK_DECL(ALTERNATE_POS_NEG, " $   123.45", "($   123.45)", " $ 3,456.78"),
	STRFMON_TC_OK_DECL(NO_CURRENCY_SYMBOL, "    123.45", "(   123.45)", "  3,456.78"),
	STRFMON_TC_OK_DECL(LEFT_JUSTIFY, " $   123.4500 ", "-$   123.4500 ", " $ 3,456.7810 "),
	STRFMON_TC_OK_DECL(RIGHT_JUSTIFY, "  $   123.4500", " -$   123.4500", "  $ 3,456.7810"),
#endif
};

ZTEST(posix_c_lib_ext, test_strfmon)
{
	TC_PRINT("%.*s\nSetting locale to %s\n%.*s\n",
		 (int)strlen("Setting locale to ") + (int)strlen(CONFIG_TEST_LOCALE_STR), HRULE,
		 CONFIG_TEST_LOCALE_STR,
		 (int)strlen("Setting locale to ") + (int)strlen(CONFIG_TEST_LOCALE_STR), HRULE);

	setlocale(LC_ALL, CONFIG_TEST_LOCALE_STR);

	ARRAY_FOR_EACH(data, j) {
		const struct test_data *const dat = &data[j];

		if (dat->tag != -1) {
			const char *const desc = strfmon_tc_desc[dat->tag];
			int desc_len = (int)strlen(desc);

			TC_PRINT("%.*s\n%s\n%.*s\n", desc_len, HRULE, desc, desc_len, HRULE);
		}

		ARRAY_FOR_EACH(input, i) {
			errno = 0;
			memset(buffer, 0, sizeof(buffer));

			ssize_t ret = strfmon(dat->s, dat->maxsize, dat->format, input[i]);
			bool success = ret == -1 ? false : true;

			zexpect_equal(success, dat->expected_success,
				      "[%zu] strfmon(%p, %zu, \"%s\") expected: %s actual: %s", j,
				      dat->s, dat->maxsize, dat->format,
				      dat->expected_success ? "success" : "failure",
				      success ? "success" : "failure");

			zexpect_equal(errno, dat->expected_errno,
				      "[%zu] strfmon errno %d (%s), expected %d (%s)", j, errno,
				      strerror(errno), dat->expected_errno,
				      strerror(dat->expected_errno));

			if (!dat->expected_success) {
				break;
			}

			zexpect_equal(ret, strlen(dat->expected_buffer[i]),
				      "[%zu] strfmon(%p, %zu, \"%s\") return %zd, expected %zu", j,
				      dat->s, dat->maxsize, dat->format, ret,
				      strlen(dat->expected_buffer[i]));

			zexpect_str_equal(
				buffer, dat->expected_buffer[i],
				"[%zu] strfmon(%p, %zu, \"%s\") output '%s', expected '%s'", j,
				dat->s, dat->maxsize, dat->format, buffer, dat->expected_buffer[i]);
		}
	}
}
