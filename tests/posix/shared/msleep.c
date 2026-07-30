/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/timeutil.h>
#include <zephyr/sys/util.h>

/* used instead of k_yield() to incur scheduler on native_sim / linux_compat */
int msleep(int ms)
{
	nanosleep(&SYS_TICKS_TO_TIMESPEC(K_MSEC(ms).ticks), NULL);
	return 0;
}

uint32_t now_ms(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec * MSEC_PER_SEC + ts.tv_nsec / NSEC_PER_MSEC;
}
