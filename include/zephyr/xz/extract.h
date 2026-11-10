/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_XZ_EXTRACT_H_
#define ZEPHYR_INCLUDE_XZ_EXTRACT_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xz_extract XZ extraction
 * @ingroup os_services
 * @{
 */

/**
 * @brief Extract an .xz stream into a file
 *
 * Decompresses @p in, a complete .xz stream, writing the data to @p path,
 * which is created (or truncated) on the file system mounted there. The
 * stream's LZMA2 dictionary is allocated from the system heap for the
 * duration, bounded by @kconfig{CONFIG_XZ_DEC_DICT_MAX}.
 *
 * @param in the stream
 * @param in_len its length in bytes
 * @param path the file to write
 * @return the number of bytes written, or a negative errno: -EINVAL for a
 *         stream that is not .xz or is corrupt, -ENOMEM for a dictionary
 *         beyond the bound or memory, -ENOTSUP for an unsupported filter,
 *         or the file system's error
 */
long xz_extract(const void *in, size_t in_len, const char *path);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_XZ_EXTRACT_H_ */
