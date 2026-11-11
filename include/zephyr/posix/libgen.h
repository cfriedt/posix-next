/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Pathname manipulation (<libgen.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/libgen.h.html">
 *      POSIX.1-2017 &lt;libgen.h&gt;</a>
 *
 * @ingroup posix_option_group_xsi_file_system
 */

#ifndef ZEPHYR_INCLUDE_POSIX_LIBGEN_H_
#define ZEPHYR_INCLUDE_POSIX_LIBGEN_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Return the final component of a pathname.
 *
 * With _GNU_SOURCE, the C library's <string.h> declares the GNU basename(), which takes a
 * const string and leaves it alone. As in picolibc, this header renames the identifier so
 * the two prototypes do not meet, while the symbol stays basename: a C library that ships
 * one is linked where the XSI_FILE_SYSTEM group is not.
 *
 * @ingroup posix_option_group_xsi_file_system
 * @param path Pathname, which may be modified in place; NULL or empty yields ".".
 * @return The last component, or "/" for a pathname of slashes.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/basename.html
 */
#undef basename
#define basename __xpg_basename
char *basename(char *path) __asm__("basename");

/**
 * @brief Return the directory portion of a pathname.
 * @ingroup posix_option_group_xsi_file_system
 * @param path Pathname, which may be modified in place; NULL or empty yields ".".
 * @return The pathname without its last component, "." when it has none.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/dirname.html
 */
char *dirname(char *path);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_LIBGEN_H_ */
