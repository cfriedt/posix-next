/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "wide_char_device_io_tests.h"

#include <stdarg.h>
#include <wchar.h>

/*
 * The standard streams are swapped onto files by descriptor. Orienting the
 * host's stdout would silence the harness, so these are Zephyr-only.
 */

ZTEST_USER(posix_wide_char_device_io, test_getwchar)
{
	posix_test_skip_if_native_libc();
	test_swap_std_in();

	zassert_equal(getwchar(), L'h');
	zassert_equal(getwchar(), L'e');
}

ZTEST_USER(posix_wide_char_device_io, test_wscanf)
{
	wchar_t word[8];
	int n;

	posix_test_skip_if_native_libc();
	test_swap_std_in();

	zassert_equal(wscanf(L"%7ls %d", word, &n), 2);
	zassert_equal(wcscmp(word, L"hello"), 0);
	zassert_equal(n, 42);
}

static int vwscanf_wrap(const wchar_t *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vwscanf(fmt, ap);
	va_end(ap);

	return ret;
}

ZTEST_USER(posix_wide_char_device_io, test_vwscanf)
{
	wchar_t word[8];
	int n;

	posix_test_skip_if_native_libc();
	test_swap_std_in();

	zassert_equal(vwscanf_wrap(L"%7ls %d", word, &n), 2);
	zassert_equal(wcscmp(word, L"hello"), 0);
	zassert_equal(n, 42);
}

ZTEST_USER(posix_wide_char_device_io, test_putwchar)
{
	posix_test_skip_if_native_libc();
	test_swap_std_out();

	zassert_equal(putwchar(L'a'), L'a');
	zassert_equal(putwchar(L'b'), L'b');
	test_restore_std();
	zassert_equal(wcscmp(test_read_out(), L"ab"), 0);
}

ZTEST_USER(posix_wide_char_device_io, test_wprintf)
{
	posix_test_skip_if_native_libc();
	test_swap_std_out();

	zassert_equal(wprintf(L"%d-%ls\n", 42, L"wide"), 8);
	test_restore_std();
	zassert_equal(wcscmp(test_read_out(), L"42-wide\n"), 0);
}

static int vwprintf_wrap(const wchar_t *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vwprintf(fmt, ap);
	va_end(ap);

	return ret;
}

ZTEST_USER(posix_wide_char_device_io, test_vwprintf)
{
	posix_test_skip_if_native_libc();
	test_swap_std_out();

	zassert_equal(vwprintf_wrap(L"%d-%ls\n", 42, L"wide"), 8);
	test_restore_std();
	zassert_equal(wcscmp(test_read_out(), L"42-wide\n"), 0);
}
