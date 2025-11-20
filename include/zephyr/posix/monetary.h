/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_POSIX_MONETARY_H_
#define ZEPHYR_INCLUDE_POSIX_MONETARY_H_

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#include <locale.h>
#include <stddef.h>
#include <sys/types.h>

#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/* locale_t is defined in locale.h */
/* size_t is defined in stddef.h */
/* ssize_t is defined in sys/types.h */

ssize_t strfmon(char *ZRESTRICT s, size_t maxsize, const char *ZRESTRICT format, ...);
ssize_t strfmon_l(char *ZRESTRICT s, size_t maxsize, locale_t locale, const char *ZRESTRICT format,
		  ...);

#ifdef __cplusplus
}
#endif

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#endif /* ZEPHYR_INCLUDE_POSIX_MONETARY_H_ */
