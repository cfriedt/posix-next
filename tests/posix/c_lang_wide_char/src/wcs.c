/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <wchar.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_c_lang_wide_char, test_wcslen)
{
	zassert_equal(wcslen(L""), 0);
	zassert_equal(wcslen(L"hello"), 5);
}

ZTEST_USER(posix_c_lang_wide_char, test_wcscpy)
{
	wchar_t buf[8];

	zassert_equal(wcscpy(buf, L"hello"), buf);
	zassert_equal(wcscmp(buf, L"hello"), 0);
	zassert_equal(wcscpy(buf, L""), buf);
	zassert_equal(buf[0], L'\0');
}

ZTEST_USER(posix_c_lang_wide_char, test_wcsncpy)
{
	wchar_t buf[8];

	/* pads with nulls, and does not terminate a truncated copy */
	wmemset(buf, L'x', ARRAY_SIZE(buf));
	zassert_equal(wcsncpy(buf, L"ab", 4), buf);
	zassert_equal(wmemcmp(buf, L"ab\0\0", 4), 0);
	zassert_equal(buf[4], L'x');
	zassert_equal(wcsncpy(buf, L"hello", 3), buf);
	zassert_equal(wmemcmp(buf, L"hel", 3), 0);
	zassert_equal(buf[3], L'\0');
}

ZTEST_USER(posix_c_lang_wide_char, test_wcscat)
{
	wchar_t buf[16] = L"hello";

	zassert_equal(wcscat(buf, L", world"), buf);
	zassert_equal(wcscmp(buf, L"hello, world"), 0);
	zassert_equal(wcscat(buf, L""), buf);
	zassert_equal(wcscmp(buf, L"hello, world"), 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_wcsncat)
{
	wchar_t buf[16] = L"hello";

	zassert_equal(wcsncat(buf, L", world", 2), buf);
	zassert_equal(wcscmp(buf, L"hello, "), 0);
	zassert_equal(wcsncat(buf, L"world", 16), buf);
	zassert_equal(wcscmp(buf, L"hello, world"), 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_wcscmp)
{
	zassert_equal(wcscmp(L"abc", L"abc"), 0);
	zassert_true(wcscmp(L"abc", L"abd") < 0);
	zassert_true(wcscmp(L"abd", L"abc") > 0);
	zassert_true(wcscmp(L"ab", L"abc") < 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_wcsncmp)
{
	zassert_equal(wcsncmp(L"abc", L"abd", 2), 0);
	zassert_true(wcsncmp(L"abc", L"abd", 3) < 0);
	zassert_equal(wcsncmp(L"abc", L"xyz", 0), 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_wcscoll)
{
	/* the C locale collates by code point */
	zassert_equal(wcscoll(L"abc", L"abc"), 0);
	zassert_true(wcscoll(L"abc", L"abd") < 0);
	zassert_true(wcscoll(L"b", L"a") > 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_wcsxfrm)
{
	wchar_t a[8];
	wchar_t b[8];

	zassert_equal(wcsxfrm(NULL, L"abc", 0), 3);
	zassert_equal(wcsxfrm(a, L"abc", ARRAY_SIZE(a)), 3);
	zassert_equal(wcsxfrm(b, L"abd", ARRAY_SIZE(b)), 3);
	zassert_true(wcscmp(a, b) < 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_wcschr)
{
	const wchar_t *s = L"hello";

	zassert_equal(wcschr(s, L'l'), &s[2]);
	zassert_equal(wcschr(s, L'\0'), &s[5]);
	zassert_is_null(wcschr(s, L'z'));
}

ZTEST_USER(posix_c_lang_wide_char, test_wcsrchr)
{
	const wchar_t *s = L"hello";

	zassert_equal(wcsrchr(s, L'l'), &s[3]);
	zassert_equal(wcsrchr(s, L'\0'), &s[5]);
	zassert_is_null(wcsrchr(s, L'z'));
}

ZTEST_USER(posix_c_lang_wide_char, test_wcsstr)
{
	const wchar_t *s = L"hello, world";

	zassert_equal(wcsstr(s, L"world"), &s[7]);
	zassert_equal(wcsstr(s, L""), s);
	zassert_is_null(wcsstr(s, L"word"));
}

ZTEST_USER(posix_c_lang_wide_char, test_wcspbrk)
{
	const wchar_t *s = L"hello, world";

	zassert_equal(wcspbrk(s, L",!"), &s[5]);
	zassert_is_null(wcspbrk(s, L"xyz"));
}

ZTEST_USER(posix_c_lang_wide_char, test_wcsspn)
{
	zassert_equal(wcsspn(L"hello", L"hel"), 4);
	zassert_equal(wcsspn(L"hello", L"xyz"), 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_wcscspn)
{
	zassert_equal(wcscspn(L"hello", L"lo"), 2);
	zassert_equal(wcscspn(L"hello", L"xyz"), 5);
}

ZTEST_USER(posix_c_lang_wide_char, test_wcstok)
{
	wchar_t buf[] = L"a,b,,c";
	wchar_t *state;

	zassert_equal(wcscmp(wcstok(buf, L",", &state), L"a"), 0);
	zassert_equal(wcscmp(wcstok(NULL, L",", &state), L"b"), 0);
	zassert_equal(wcscmp(wcstok(NULL, L",", &state), L"c"), 0);
	zassert_is_null(wcstok(NULL, L",", &state));
}

ZTEST_USER(posix_c_lang_wide_char, test_wmemchr)
{
	const wchar_t s[] = L"hel\0lo";

	zassert_equal(wmemchr(s, L'l', 6), &s[2]);
	zassert_equal(wmemchr(s, L'o', 6), &s[5]);
	zassert_is_null(wmemchr(s, L'o', 5));
}

ZTEST_USER(posix_c_lang_wide_char, test_wmemcmp)
{
	zassert_equal(wmemcmp(L"abc", L"abd", 2), 0);
	zassert_true(wmemcmp(L"abc", L"abd", 3) < 0);
	zassert_true(wmemcmp(L"abd", L"abc", 3) > 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_wmemcpy)
{
	wchar_t buf[4] = L"xxxx";

	zassert_equal(wmemcpy(buf, L"ab", 2), buf);
	zassert_equal(wmemcmp(buf, L"abxx", 4), 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_wmemmove)
{
	wchar_t buf[] = L"abcdef";

	/* overlapping in both directions */
	zassert_equal(wmemmove(&buf[2], &buf[0], 4), &buf[2]);
	zassert_equal(wcscmp(buf, L"ababcd"), 0);
	zassert_equal(wmemmove(&buf[0], &buf[2], 4), &buf[0]);
	zassert_equal(wcscmp(buf, L"abcdcd"), 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_wmemset)
{
	wchar_t buf[4] = L"xxxx";

	zassert_equal(wmemset(buf, L'a', 3), buf);
	zassert_equal(wmemcmp(buf, L"aaax", 4), 0);
}
