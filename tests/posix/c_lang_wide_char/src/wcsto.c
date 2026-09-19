/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <wchar.h>

#include <zephyr/ztest.h>

#define INT_TEST(fn, type, base_ok, big, big_val)                                                  \
	ZTEST_USER(posix_c_lang_wide_char, test_##fn)                                              \
	{                                                                                          \
		const wchar_t *s = L"  42xyz";                                                     \
		const wchar_t *none = L"zzz";                                                      \
		wchar_t *end;                                                                      \
                                                                                                   \
		zassert_equal(fn(s, &end, 10), (type)42);                                          \
		zassert_equal(end, &s[4]);                                                         \
		zassert_equal(fn(L"0x1f", &end, 16), (type)31);                                    \
		zassert_equal(fn(L"0x1f", &end, 0), (type)31);                                     \
		zassert_equal(fn(L"017", NULL, 0), (type)15);                                      \
		zassert_equal(fn(base_ok, &end, 10), (type)-7);                                    \
		zassert_equal(fn(none, &end, 10), (type)0);                                        \
		zassert_equal(end, none);                                                          \
		errno = 0;                                                                         \
		zassert_equal(fn(big, NULL, 10), big_val);                                         \
		zassert_equal(errno, ERANGE);                                                      \
	}

INT_TEST(wcstol, long, L"-7", L"99999999999999999999999", LONG_MAX)
INT_TEST(wcstoll, long long, L"-7", L"99999999999999999999999", LLONG_MAX)
INT_TEST(wcstoimax, intmax_t, L"-7", L"99999999999999999999999", INTMAX_MAX)
INT_TEST(wcstoul, unsigned long, L"-7", L"99999999999999999999999", ULONG_MAX)
INT_TEST(wcstoull, unsigned long long, L"-7", L"99999999999999999999999", ULLONG_MAX)
INT_TEST(wcstoumax, uintmax_t, L"-7", L"99999999999999999999999", UINTMAX_MAX)

/* compared as double: the 32-bit targets ship no long double comparison helpers */
#define FLOAT_TEST(fn, big)                                                                        \
	ZTEST_USER(posix_c_lang_wide_char, test_##fn)                                              \
	{                                                                                          \
		const wchar_t *s = L"  1.5e1xyz";                                                  \
		const wchar_t *none = L"xyz";                                                      \
		wchar_t *end;                                                                      \
                                                                                                   \
		zassert_equal((double)fn(s, &end), 15.0);                                          \
		zassert_equal(end, &s[7]);                                                         \
		zassert_equal((double)fn(L"-0.25", NULL), -0.25);                                  \
		zassert_equal((double)fn(L"0x10", NULL), 16.0);                                    \
		zassert_true(isinf((double)fn(L"inf", NULL)));                                     \
		zassert_true(isnan((double)fn(L"nan", NULL)));                                     \
		zassert_equal((double)fn(none, &end), 0.0);                                        \
		zassert_equal(end, none);                                                          \
		errno = 0;                                                                         \
		zassert_true(isinf((double)fn(big, NULL)));                                        \
		zassert_equal(errno, ERANGE);                                                      \
	}

FLOAT_TEST(wcstod, L"1e999")
FLOAT_TEST(wcstof, L"1e99")

ZTEST_USER(posix_c_lang_wide_char, test_wcstold)
{
	const wchar_t *s = L"  1.5e1xyz";
	const wchar_t *none = L"xyz";
	wchar_t *end;

	if (IS_ENABLED(CONFIG_X86) && !IS_ENABLED(CONFIG_X86_64)) {
		/* the IA-32 soft-float stubs oops on long double arithmetic */
		ztest_test_skip();
	}

	zassert_equal((double)wcstold(s, &end), 15.0);
	zassert_equal(end, &s[7]);
	zassert_equal((double)wcstold(L"-0.25", NULL), -0.25);
	zassert_equal((double)wcstold(L"0x10", NULL), 16.0);
	zassert_true(isinf((double)wcstold(L"inf", NULL)));
	zassert_true(isnan((double)wcstold(L"nan", NULL)));
	zassert_equal((double)wcstold(none, &end), 0.0);
	zassert_equal(end, none);
	errno = 0;
	zassert_true(isinf((double)wcstold(L"1e99999", NULL)));
	zassert_equal(errno, ERANGE);
}
