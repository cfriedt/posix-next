/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdarg.h>
#include <time.h>
#include <wchar.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_c_lang_wide_char, test_swprintf)
{
	wchar_t buf[32];

	int ret;

	zassert_equal(swprintf(buf, ARRAY_SIZE(buf), L"%d-%ls-%lc", 42, L"wide", L'x'), 9);
	zassert_equal(wcscmp(buf, L"42-wide-x"), 0);

	/* a result that does not fit is an error; Picolibc reports the length it needed instead */
	ret = swprintf(buf, 3, L"%d", 12345);
	zassert_true((ret < 0) || (IS_ENABLED(CONFIG_PICOLIBC) && (ret == 5)),
		     "swprintf: %d", ret);
}

static int vswprintf_wrap(wchar_t *buf, size_t n, const wchar_t *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vswprintf(buf, n, fmt, ap);
	va_end(ap);

	return ret;
}

ZTEST_USER(posix_c_lang_wide_char, test_vswprintf)
{
	wchar_t buf[32];

	int ret;

	zassert_equal(vswprintf_wrap(buf, ARRAY_SIZE(buf), L"%d-%ls", 42, L"wide"), 7);
	zassert_equal(wcscmp(buf, L"42-wide"), 0);

	ret = vswprintf_wrap(buf, 3, L"%d", 12345);
	zassert_true((ret < 0) || (IS_ENABLED(CONFIG_PICOLIBC) && (ret == 5)),
		     "vswprintf: %d", ret);
}

ZTEST_USER(posix_c_lang_wide_char, test_swscanf)
{
	wchar_t word[8];
	int n;

	zassert_equal(swscanf(L"42 wide", L"%d %7ls", &n, word), 2);
	zassert_equal(n, 42);
	zassert_equal(wcscmp(word, L"wide"), 0);
	zassert_equal(swscanf(L"x", L"%d", &n), 0);
	zassert_equal(swscanf(L"", L"%d", &n), EOF);
}

static int vswscanf_wrap(const wchar_t *s, const wchar_t *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vswscanf(s, fmt, ap);
	va_end(ap);

	return ret;
}

ZTEST_USER(posix_c_lang_wide_char, test_vswscanf)
{
	wchar_t word[8];
	int n;

	zassert_equal(vswscanf_wrap(L"42 wide", L"%d %7ls", &n, word), 2);
	zassert_equal(n, 42);
	zassert_equal(wcscmp(word, L"wide"), 0);
	zassert_equal(vswscanf_wrap(L"", L"%d", &n), EOF);
}

ZTEST_USER(posix_c_lang_wide_char, test_wcsftime)
{
	/* 2009-02-13 23:31:30 UTC */
	const struct tm tm = {
		.tm_year = 109, .tm_mon = 1, .tm_mday = 13, .tm_hour = 23, .tm_min = 31,
		.tm_sec = 30, .tm_wday = 5, .tm_yday = 43,
	};
	wchar_t buf[32];

	zassert_equal(wcsftime(buf, ARRAY_SIZE(buf), L"%Y-%m-%d %H:%M:%S", &tm), 19);
	zassert_equal(wcscmp(buf, L"2009-02-13 23:31:30"), 0);
	/* a result that does not fit yields 0 */
	zassert_equal(wcsftime(buf, 4, L"%Y-%m-%d", &tm), 0);
}
