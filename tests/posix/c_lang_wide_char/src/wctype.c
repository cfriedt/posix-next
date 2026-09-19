/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <wchar.h>
#include <wctype.h>

#include <zephyr/ztest.h>

/* the C locale: only the ASCII classes are known */
#define CLASS_TEST(fn, yes, no)                                                                    \
	ZTEST_USER(posix_c_lang_wide_char, test_##fn)                                              \
	{                                                                                          \
		zassert_true(fn(yes) != 0);                                                        \
		zassert_true(fn(no) == 0);                                                         \
		zassert_true(fn(WEOF) == 0);                                                       \
	}

CLASS_TEST(iswalnum, L'7', L' ')
CLASS_TEST(iswalpha, L'q', L'7')
CLASS_TEST(iswblank, L'\t', L'\n')
CLASS_TEST(iswcntrl, L'\n', L'a')
CLASS_TEST(iswdigit, L'0', L'a')
CLASS_TEST(iswgraph, L'!', L' ')
CLASS_TEST(iswlower, L'a', L'A')
CLASS_TEST(iswprint, L' ', L'\n')
CLASS_TEST(iswpunct, L'.', L'a')
CLASS_TEST(iswspace, L' ', L'a')
CLASS_TEST(iswupper, L'A', L'a')
CLASS_TEST(iswxdigit, L'f', L'g')

ZTEST_USER(posix_c_lang_wide_char, test_iswctype)
{
	zassert_true(iswctype(L'a', wctype("alpha")) != 0);
	zassert_true(iswctype(L'1', wctype("alpha")) == 0);
	zassert_true(iswctype(L'1', wctype("digit")) != 0);
	zassert_true(iswctype(WEOF, wctype("digit")) == 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_wctype)
{
	zassert_true(wctype("alpha") != 0);
	zassert_true(wctype("space") != 0);
	zassert_equal(wctype("no such class"), 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_towlower)
{
	zassert_equal(towlower(L'A'), L'a');
	zassert_equal(towlower(L'a'), L'a');
	zassert_equal(towlower(L'1'), L'1');
	zassert_equal(towlower(WEOF), WEOF);
}

ZTEST_USER(posix_c_lang_wide_char, test_towupper)
{
	zassert_equal(towupper(L'a'), L'A');
	zassert_equal(towupper(L'A'), L'A');
	zassert_equal(towupper(L'1'), L'1');
	zassert_equal(towupper(WEOF), WEOF);
}

ZTEST_USER(posix_c_lang_wide_char, test_wctrans)
{
	zassert_true(wctrans("tolower") != 0);
	zassert_true(wctrans("toupper") != 0);
	zassert_equal(wctrans("no such mapping"), 0);
}

ZTEST_USER(posix_c_lang_wide_char, test_towctrans)
{
	zassert_equal(towctrans(L'a', wctrans("toupper")), L'A');
	zassert_equal(towctrans(L'A', wctrans("tolower")), L'a');
	zassert_equal(towctrans(L'1', wctrans("toupper")), L'1');
}
