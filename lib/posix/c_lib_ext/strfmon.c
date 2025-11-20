/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <ctype.h>
#include <locale.h>
#include <monetary.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <zephyr/toolchain.h>

struct strfmon_format_spec {
	int8_t field_width;      /* overall field width (like printf) */
	int8_t left_precision;   /* minimum digits to left of decimal (#) */
	int8_t right_precision;  /* digits to right of decimal (default locale) */
	char fill_char;          /* general pad char for field width */
	char numeric_fill;       /* fill char for left precision (from =X) */
	char int_curr_symbol[4]; /* international currency symbol (use e.g. "USD" instead of "$") */
	bool use_numeric_fill: 1;    /* whether numeric_fill is active (=X) */
	bool use_international: 1;   /* %i instead of %n (use int_curr_symbol) */
	bool left_justify: 1;        /* - flag */
	bool disable_grouping: 1;    /* ^ flag */
	bool use_currency_symbol: 1; /* ! flag negation: if false, suppress symbol */
	bool use_parens: 1;          /* ( flag: negative numbers in parentheses */
	bool force_sign: 1;          /* + flag: force sign for positives */
};

static int8_t read_number(const char **pp)
{
	int8_t value = 0;
	bool found = false;
	const char *p = *pp;

	while (isdigit((unsigned char)*p)) {
		found = true;
		value = (int8_t)(value * 10 + (*p - '0'));
		++p;
	}

	if (!found) {
		return -1;
	}

	*pp = p;
	return value;
}

static bool strfmon_parse_format(const char **fmtp, const struct lconv *lc,
				 struct strfmon_format_spec *spec)
{
	const char *p = *fmtp;

	*spec = (struct strfmon_format_spec){
		.use_currency_symbol = true,
		.fill_char = ' ',
		.numeric_fill = ' ',
		.right_precision = -1,
	};

	for (bool done_flags = false; !done_flags && (*p != '\0');) {
		switch (*p) {
		case '^':
			spec->disable_grouping = true;
			++p;
			break;
		case '+':
			spec->force_sign = true;
			++p;
			break;
		case '(':
			spec->use_parens = true;
			++p;
			break;
		case '!':
			spec->use_currency_symbol = false;
			++p;
			break;
		case '-':
			spec->left_justify = true;
			++p;
			break;
		case '=':
			++p;
			if (*p == '\0') {
				return false;
			}
			spec->use_numeric_fill = true;
			spec->numeric_fill = *p;
			++p;
			break;

		default:
			done_flags = true;
			break;
		}
	}

	/* field width */
	spec->field_width = read_number(&p);
	if (spec->field_width < 0) {
		spec->field_width = 0;
	}

	/* left precision (#) */
	if (*p == '#') {
		++p;
		spec->left_precision = read_number(&p);
		if (spec->left_precision < 0) {
			/* # must be followed by digits */
			return false;
		}
	}

	/* Right precision (.R) */
	if (*p == '.') {
		++p;
		spec->right_precision = read_number(&p);
		if (spec->right_precision < 0) {
			/* . must be followed by digits */
			return false;
		}
	}

	/* conversion specifier */
	if (*p == 'i') {
		spec->use_international = true;
		if (spec->right_precision < 0) {
			spec->right_precision = lc->int_frac_digits;
		}
	} else if (*p == 'n') {
		spec->use_international = false;
		if ((spec->right_precision < 0)) {
			spec->right_precision = lc->frac_digits;
		}
	} else {
		return false;
	}

	if (spec->right_precision < 0) {
		/* default if unspecified */
		spec->right_precision = 2;
	}

	/* number of characters consumed */
	*fmtp = p + 1;

	return true;
}

static void putc_if_space(char ch, char **ptr, size_t *size)
{
	if (*size == 0) {
		return;
	}

	**ptr = ch;
	*ptr = *ptr + 1;
	*size = *size - 1;
}

/* Implementation for the "C" locale */
static void strfmon_one(char **sp, size_t *maxsizep, const struct lconv *lc,
			const struct strfmon_format_spec *spec, double val)
{
	char *s = *sp;
	size_t maxsize = *maxsizep;

	/* Simplifications for "C" locale:
	 * - no grouping
	 * - no currency symbol
	 *
	 * Behavior implemented:
	 * - banker's rounding (round half to even) for right_precision
	 * - no temporary buffers; write directly to s (respecting maxsize)
	 */

	int right = spec->right_precision;

	if (right < 0) {
		right = 2; /* fallback if unspecified; typical default */
	}

	/* sign handling */
	bool negative = (val < 0.0);
	/* treat -0.0 as non-negative for simplicity, but keep negative if val < 0 */
	double absval = negative ? -val : val;

	/* compute integer scale (iscale = 10^right) as uint64_t */
	uint64_t iscale = 1;

	for (int i = 0; i < right; ++i) {
		iscale *= 10ULL;
	}

	/* scaled value = absval * iscale
	 * Use double to form scaled, then convert to uint64_t for integer rounding.
	 * This avoids math.h calls while still providing deterministic rounding.
	 */
	double dscale = (double)iscale;
	double scaled = absval * dscale;

	/* split integer and fractional parts at the scaled level */
	uint64_t scaled_int = (uint64_t)scaled;
	double frac = scaled - (double)scaled_int;

	/* banker (round half to even) rounding:
	 * - if frac > 0.5 -> round up
	 * - if frac < 0.5 -> leave
	 * - if exactly 0.5 -> round to make scaled_int even
	 *
	 * Use an epsilon to avoid floating-point rounding noise.
	 */
	const double eps = 1e-12;

	if (frac > 0.5 + eps) {
		++scaled_int;
	} else if ((frac > 0.5 - eps) && (frac < 0.5 + eps)) {
		/* tie: round to even */
		if (scaled_int & 1ULL) {
			++scaled_int;
		}
	} else {
		/* frac < 0.5: do nothing (truncate) */
	}

	/* obtain integer and fractional integer parts after rounding */
	uint64_t int_part = scaled_int / iscale;
	uint64_t frac_part = scaled_int % iscale;

	/* count integer digits */
	int int_digits = 1;

	{
		uint64_t t = int_part;

		if (t == 0) {
			int_digits = 1;
		} else {
			int_digits = 0;
			while (t > 0) {
				++int_digits;
				t /= 10ULL;
			}
		}
	}

	/* left precision (minimum digits to left of decimal) */
	int left_prec = spec->left_precision;

	if (left_prec < 0) {
		left_prec = 0;
	}

	int int_display_digits = int_digits;

	if (left_prec > int_display_digits) {
		int_display_digits = left_prec;
	}

	/* compute length of fractional part in characters */
	int frac_chars = (right > 0) ? right : 0;

	/* compute sign/parentheses length */
	int sign_lead = 0, sign_trail = 0;

	if (negative) {
		if (spec->use_parens) {
			sign_lead = 1;  /* '(' */
			sign_trail = 1; /* ')' */
		} else {
			sign_lead = 1; /* '-' */
		}
	} else {
		if (spec->force_sign || left_prec > 0) {
			sign_lead = 1; /* '+' or space for alignment */
		}
	}

	/* total length for number itself (without field padding) */
	int number_len =
		sign_lead + int_display_digits + (frac_chars ? (1 + frac_chars) : 0) + sign_trail;

	/* field width padding */
	int field_width = spec->field_width;

	if (field_width < 0) {
		field_width = 0;
	}

	int pad = 0;

	if (field_width > number_len) {
		pad = field_width - number_len;
	}

	/* If left_justify is false, write leading padding (fill_char) */
	if (!spec->left_justify) {
		for (int i = 0; i < pad; ++i) {
			putc_if_space(spec->fill_char, &s, &maxsize);
		}
	}

	/* write sign lead */
	if (sign_lead) {
		if (negative && spec->use_parens) {
			putc_if_space('(', &s, &maxsize);
		} else if (negative) {
			putc_if_space('-', &s, &maxsize);
		} else if (spec->force_sign) {
			putc_if_space('+', &s, &maxsize);
		} else {
			/* space for alignment when left_prec > 0 */
			putc_if_space(' ', &s, &maxsize);
		}
	}

	/* integer left padding due to left_precision: use numeric_fill */
	int int_pad = int_display_digits - int_digits;

	for (int i = 0; i < int_pad; ++i) {
		putc_if_space(spec->numeric_fill, &s, &maxsize);
	}

	/* Write integer digits from most-significant to least */
	{
		/* compute divisor = 10^(int_digits-1) */
		uint64_t divisor = 1;

		for (int i = 1; i < int_digits; ++i) {
			divisor *= 10ULL;
		}

		/* if int_part == 0 we still need to write '0' (but we've accounted for padding
		 * already)
		 */
		if (int_part == 0) {
			putc_if_space('0', &s, &maxsize);
			/* if int_display_digits > 1, leading numeric_fill digits already emitted
			 * above
			 */
		} else {
			uint64_t cur = int_part;

			/* If left_precision forced more digits than int_digits, we still only write
			 * the actual digits; extra leading digits were written as numeric_fill
			 * above.
			 */
			while (divisor > 0) {
				uint64_t d = cur / divisor;

				putc_if_space((char)('0' + (int)(d % 10ULL)), &s, &maxsize);
				divisor /= 10ULL;
			}
		}
	}

	/* fractional part */
	if (frac_chars > 0) {
		putc_if_space('.', &s, &maxsize);
		/* Write frac_chars digits, leading zeros if needed. */
		/* compute divisor = 10^(frac_chars-1) */
		uint64_t ddiv = 1;

		for (int i = 1; i < frac_chars; ++i) {
			ddiv *= 10ULL;
		}

		uint64_t curf = frac_part;

		/* If frac_part has fewer digits than frac_chars, the leading zeros are written
		 * naturally by divisor computation
		 */
		for (int i = 0; i < frac_chars; ++i) {
			uint64_t digit = curf / ddiv;

			putc_if_space((char)('0' + (int)digit), &s, &maxsize);
			curf %= ddiv;
			if (ddiv > 0) {
				ddiv /= 10ULL;
			}
		}
	}

	/* sign trail (closing parenthesis) */
	if (sign_trail) {
		putc_if_space(')', &s, &maxsize);
	}

	/* trailing padding if left_justify */
	if (spec->left_justify) {
		for (int i = 0; i < pad; ++i) {
			putc_if_space(spec->fill_char, &s, &maxsize);
		}
	}

	/* update pointers and remaining size */
	*sp = s;
	*maxsizep = maxsize;
}

ssize_t strfmon(char *ZRESTRICT s, size_t maxsize, const char *ZRESTRICT format, ...)
{
	char ch;
	char prev;
	va_list args;
	char *s_backup = s;
	struct lconv *const lc = localeconv();

	if (maxsize == 0) {
		errno = E2BIG;
		return -1;
	}

	va_start(args, format);

	for (prev = '\0'; *format != '\0'; prev = ch) {
		ch = *format;

		if ((ch == '%') && (prev != '%') && (*(format + 1) == '\0')) {
			/* an incomplete format specifier (trailing '%') */
			errno = EINVAL;
			/* dirty way to set the return value to -1 */
			s_backup = s + 1;
			break;
		} else if (((ch == '%') && (prev == '%')) || ((ch != '%') && (prev != '%'))) {
			/* regular character (or escaped '%') is copied to the output */
			if (maxsize > 0) {
				*s++ = ch;
				--maxsize;
			}
			++format;
			continue;
		} else if (ch == '%') {
			/* the next character is part of a format specifier */
			++format;
			continue;
		}

		/* parse format specifier */
		struct strfmon_format_spec spec;

		if (!strfmon_parse_format((const char **)&format, lc, &spec)) {
			errno = EINVAL;
			/* dirty way to set the return value to -1 */
			s_backup = s + 1;
			break;
		}

		/* read argument */
		double val = va_arg(args, double);

		/* reset '%' state machine */
		prev = '\0';

		/* format one argument */
		strfmon_one((char **)&s, &maxsize, lc, &spec, val);
	}

	va_end(args);

	/* null-terminate the string */
	if (maxsize > 0) {
		*s = '\0';
	}

	return s - s_backup;
}
