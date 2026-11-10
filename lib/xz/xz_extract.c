/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdint.h>

#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>
#include <zephyr/xz/extract.h>

#include "xz.h"

#define OUT_CHUNK 4096

long xz_extract(const void *in, size_t in_len, const char *path)
{
	struct xz_dec *dec;
	struct fs_file_t file;
	uint8_t *out;
	struct xz_buf b;
	enum xz_ret xr = XZ_OK;
	long total = 0;
	int rc;

	out = k_malloc(OUT_CHUNK);
	if (out == NULL) {
		return -ENOMEM;
	}
	xz_crc32_init();
	dec = xz_dec_init(XZ_DYNALLOC, CONFIG_XZ_DEC_DICT_MAX);
	if (dec == NULL) {
		k_free(out);
		return -ENOMEM;
	}
	fs_file_t_init(&file);
	rc = fs_open(&file, path, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	if (rc < 0) {
		xz_dec_end(dec);
		k_free(out);
		return rc;
	}

	b.in = in;
	b.in_pos = 0;
	b.in_size = in_len;
	b.out = out;
	b.out_size = OUT_CHUNK;
	do {
		b.out_pos = 0;
		xr = xz_dec_run(dec, &b);
		if (b.out_pos > 0) {
			ssize_t n = fs_write(&file, out, b.out_pos);

			if (n < 0) {
				rc = (int)n;
				break;
			}
			if ((size_t)n != b.out_pos) {
				rc = -ENOSPC;
				break;
			}
			total += (long)n;
		}
	} while (xr == XZ_OK);

	if (rc >= 0) {
		switch (xr) {
		case XZ_STREAM_END:
			break;
		case XZ_MEMLIMIT_ERROR:
		case XZ_MEM_ERROR:
			rc = -ENOMEM;
			break;
		case XZ_OPTIONS_ERROR:
		case XZ_UNSUPPORTED_CHECK:
			rc = -ENOTSUP;
			break;
		default:
			rc = -EINVAL;
			break;
		}
	}
	(void)fs_close(&file);
	xz_dec_end(dec);
	k_free(out);

	return (rc < 0) ? rc : total;
}
