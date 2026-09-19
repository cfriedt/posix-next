/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "wide_char_device_io_tests.h"

#include <stdarg.h>
#include <wchar.h>

ZTEST_USER(posix_wide_char_device_io, test_fwide)
{
	FILE *f = test_open_in();

	/* a byte operation leaves the stream byte-oriented, a wide request makes it wide */
	zassert_true(fgetc(f) != EOF);
	zassert_true(fwide(f, 0) < 0);
	zassert_ok(fclose(f));

	f = test_open_out();
	zassert_true(fwide(f, 1) > 0);
	zassert_true(fwide(f, 0) > 0);
	zassert_ok(fclose(f));
}

ZTEST_USER(posix_wide_char_device_io, test_fgetwc)
{
	FILE *f = test_open_in();

	zassert_equal(fgetwc(f), L'h');
	zassert_equal(fgetwc(f), L'e');
	zassert_true(fwide(f, 0) > 0);
	while (fgetwc(f) != WEOF) {
	}
	zassert_equal(fgetwc(f), WEOF);
	/* Picolibc leaves the end-of-file indicator clear on wide reads */
	if (!IS_ENABLED(CONFIG_PICOLIBC)) {
		zassert_true(feof(f));
	}
	zassert_ok(fclose(f));
}

ZTEST_USER(posix_wide_char_device_io, test_getwc)
{
	FILE *f = test_open_in();

	zassert_equal(getwc(f), L'h');
	zassert_equal(getwc(f), L'e');
	zassert_ok(fclose(f));
}

ZTEST_USER(posix_wide_char_device_io, test_ungetwc)
{
	FILE *f = test_open_in();

	zassert_equal(fgetwc(f), L'h');
	zassert_equal(ungetwc(L'X', f), L'X');
	zassert_equal(fgetwc(f), L'X');
	zassert_equal(fgetwc(f), L'e');
	zassert_equal(ungetwc(WEOF, f), WEOF);
	zassert_ok(fclose(f));
}

ZTEST_USER(posix_wide_char_device_io, test_fgetws)
{
	FILE *f = test_open_in();
	wchar_t buf[32];

	/* a line at a time, a bounded read, then the end */
	zassert_equal(fgetws(buf, ARRAY_SIZE(buf), f), buf);
	zassert_equal(wcscmp(buf, L"hello 42\n"), 0);
	zassert_equal(fgetws(buf, 4, f), buf);
	zassert_equal(wcscmp(buf, L"sec"), 0);
	zassert_equal(fgetws(buf, ARRAY_SIZE(buf), f), buf);
	zassert_equal(wcscmp(buf, L"ond line\n"), 0);
	zassert_is_null(fgetws(buf, ARRAY_SIZE(buf), f));
	if (!IS_ENABLED(CONFIG_PICOLIBC)) {
		zassert_true(feof(f));
	}
	zassert_ok(fclose(f));
}

ZTEST_USER(posix_wide_char_device_io, test_fputwc)
{
	FILE *f = test_open_out();

	zassert_equal(fputwc(L'a', f), L'a');
	zassert_equal(fputwc(L'b', f), L'b');
	zassert_true(fwide(f, 0) > 0);
	zassert_ok(fclose(f));
	zassert_equal(wcscmp(test_read_out(), L"ab"), 0);
}

ZTEST_USER(posix_wide_char_device_io, test_putwc)
{
	FILE *f = test_open_out();

	zassert_equal(putwc(L'x', f), L'x');
	zassert_equal(putwc(L'y', f), L'y');
	zassert_ok(fclose(f));
	zassert_equal(wcscmp(test_read_out(), L"xy"), 0);
}

ZTEST_USER(posix_wide_char_device_io, test_fputws)
{
	FILE *f = test_open_out();

	zassert_true(fputws(L"wide ", f) >= 0);
	zassert_true(fputws(L"string\n", f) >= 0);
	zassert_ok(fclose(f));
	zassert_equal(wcscmp(test_read_out(), L"wide string\n"), 0);
}

ZTEST_USER(posix_wide_char_device_io, test_fwprintf)
{
	FILE *f = test_open_out();

	zassert_equal(fwprintf(f, L"%d-%ls-%lc\n", 42, L"wide", L'x'), 10);
	zassert_ok(fclose(f));
	zassert_equal(wcscmp(test_read_out(), L"42-wide-x\n"), 0);
}

static int vfwprintf_wrap(FILE *f, const wchar_t *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vfwprintf(f, fmt, ap);
	va_end(ap);

	return ret;
}

ZTEST_USER(posix_wide_char_device_io, test_vfwprintf)
{
	FILE *f = test_open_out();

	zassert_equal(vfwprintf_wrap(f, L"%d-%ls\n", 42, L"wide"), 8);
	zassert_ok(fclose(f));
	zassert_equal(wcscmp(test_read_out(), L"42-wide\n"), 0);
}

ZTEST_USER(posix_wide_char_device_io, test_fwscanf)
{
	FILE *f = test_open_in();
	wchar_t word[8];
	int n;

	zassert_equal(fwscanf(f, L"%7ls %d", word, &n), 2);
	zassert_equal(wcscmp(word, L"hello"), 0);
	zassert_equal(n, 42);
	/* the second line is not a number */
	zassert_equal(fwscanf(f, L"%d", &n), 0);
	zassert_ok(fclose(f));
}

static int vfwscanf_wrap(FILE *f, const wchar_t *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vfwscanf(f, fmt, ap);
	va_end(ap);

	return ret;
}

ZTEST_USER(posix_wide_char_device_io, test_vfwscanf)
{
	FILE *f = test_open_in();
	wchar_t word[8];
	int n;

	zassert_equal(vfwscanf_wrap(f, L"%7ls %d", word, &n), 2);
	zassert_equal(wcscmp(word, L"hello"), 0);
	zassert_equal(n, 42);
	zassert_ok(fclose(f));
}
