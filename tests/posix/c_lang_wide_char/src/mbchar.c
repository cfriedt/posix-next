/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_c_lang_wide_char, test_btowc)
{
	zassert_equal(btowc('a'), L'a');
	zassert_equal(btowc('\0'), L'\0');
	zassert_equal(btowc(EOF), WEOF);
}

ZTEST_USER(posix_c_lang_wide_char, test_wctob)
{
	zassert_equal(wctob(L'a'), 'a');
	zassert_equal(wctob(L'\0'), '\0');
	zassert_equal(wctob(WEOF), EOF);
}

ZTEST_USER(posix_c_lang_wide_char, test_mblen)
{
	zassert_equal(mblen(NULL, 0), 0);
	zassert_equal(mblen("a", 1), 1);
	zassert_equal(mblen("", 1), 0);
	zassert_equal(mblen("a", 0), -1);
}

ZTEST_USER(posix_c_lang_wide_char, test_mbtowc)
{
	wchar_t wc = L'x';

	zassert_equal(mbtowc(NULL, NULL, 0), 0);
	zassert_equal(mbtowc(&wc, "a", 1), 1);
	zassert_equal(wc, L'a');
	zassert_equal(mbtowc(&wc, "", 1), 0);
	zassert_equal(wc, L'\0');
	zassert_equal(mbtowc(&wc, "a", 0), -1);
}

ZTEST_USER(posix_c_lang_wide_char, test_wctomb)
{
	char buf[16];

	zassert_equal(wctomb(buf, L'a'), 1);
	zassert_equal(buf[0], 'a');
	zassert_equal(wctomb(buf, L'\0'), 1);
	zassert_equal(buf[0], '\0');
	zassert_equal(wctomb(NULL, L'a'), 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_mbsinit)
{
	mbstate_t st;

	zassert_true(mbsinit(NULL) != 0);
	memset(&st, 0, sizeof(st));
	zassert_true(mbsinit(&st) != 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_mbrlen)
{
	mbstate_t st;

	memset(&st, 0, sizeof(st));
	zassert_equal(mbrlen("a", 1, &st), 1);
	zassert_equal(mbrlen("", 1, &st), 0);
	zassert_equal(mbrlen("a", 0, &st), (size_t)-2);
	zassert_equal(mbrlen("a", 1, NULL), 1);
}

ZTEST_USER(posix_c_lang_wide_char, test_mbrtowc)
{
	mbstate_t st;
	wchar_t wc = L'x';

	memset(&st, 0, sizeof(st));
	zassert_equal(mbrtowc(&wc, "a", 1, &st), 1);
	zassert_equal(wc, L'a');
	zassert_equal(mbrtowc(&wc, "", 1, &st), 0);
	zassert_equal(wc, L'\0');
	zassert_equal(mbrtowc(&wc, "a", 0, &st), (size_t)-2);
	zassert_equal(mbrtowc(NULL, "a", 1, NULL), 1);
}

ZTEST_USER(posix_c_lang_wide_char, test_wcrtomb)
{
	char buf[16];
	mbstate_t st;

	memset(&st, 0, sizeof(st));
	zassert_equal(wcrtomb(buf, L'a', &st), 1);
	zassert_equal(buf[0], 'a');
	zassert_equal(wcrtomb(buf, L'\0', &st), 1);
	zassert_equal(buf[0], '\0');
	zassert_equal(wcrtomb(NULL, L'a', NULL), 1);
}

ZTEST_USER(posix_c_lang_wide_char, test_mbstowcs)
{
	wchar_t wcs[8];

	zassert_equal(mbstowcs(wcs, "abc", ARRAY_SIZE(wcs)), 3);
	zassert_equal(wcscmp(wcs, L"abc"), 0);
	/* a short destination is not terminated */
	wcs[2] = L'x';
	zassert_equal(mbstowcs(wcs, "abc", 2), 2);
	zassert_equal(wcs[2], L'x');
}

ZTEST_USER(posix_c_lang_wide_char, test_wcstombs)
{
	char buf[8];

	zassert_equal(wcstombs(buf, L"abc", sizeof(buf)), 3);
	zassert_str_equal(buf, "abc");
	buf[2] = 'x';
	zassert_equal(wcstombs(buf, L"abc", 2), 2);
	zassert_equal(buf[2], 'x');
}

ZTEST_USER(posix_c_lang_wide_char, test_mbsrtowcs)
{
	const char *src = "abc";
	wchar_t wcs[8];
	mbstate_t st;

	memset(&st, 0, sizeof(st));
	zassert_equal(mbsrtowcs(NULL, &src, 0, &st), 3);
	zassert_equal(mbsrtowcs(wcs, &src, ARRAY_SIZE(wcs), &st), 3);
	zassert_equal(wcscmp(wcs, L"abc"), 0);
	/* the whole string converted */
	zassert_is_null(src);

	/* a short destination stops with src at the rest */
	src = "abc";
	zassert_equal(mbsrtowcs(wcs, &src, 2, &st), 2);
	zassert_str_equal(src, "c");
}

ZTEST_USER(posix_c_lang_wide_char, test_wcsrtombs)
{
	const wchar_t *src = L"abc";
	char buf[8];
	mbstate_t st;

	memset(&st, 0, sizeof(st));
	zassert_equal(wcsrtombs(NULL, &src, 0, &st), 3);
	zassert_equal(wcsrtombs(buf, &src, sizeof(buf), &st), 3);
	zassert_str_equal(buf, "abc");
	zassert_is_null(src);

	src = L"abc";
	zassert_equal(wcsrtombs(buf, &src, 2, &st), 2);
	zassert_equal(wcscmp(src, L"c"), 0);
}
