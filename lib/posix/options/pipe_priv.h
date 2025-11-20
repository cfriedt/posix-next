/*
 * Copyright (c) 2025 Atym, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_OPTIONS_PIPE_PRIV_H_
#define ZEPHYR_LIB_POSIX_OPTIONS_PIPE_PRIV_H_

#include <stdint.h>
#include <stdbool.h>

#include <zephyr/kernel.h>

struct posix_pipe_desc {
    struct k_pipe *pipe;
    int flags;
    uint8_t *ring_buffer;
    bool one_end_closed: 1;
	bool used: 1;
};

#endif
