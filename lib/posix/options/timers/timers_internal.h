/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_OPTIONS_TIMERS_TIMERS_INTERNAL_H_
#define ZEPHYR_LIB_POSIX_OPTIONS_TIMERS_TIMERS_INTERNAL_H_

#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/toolchain.h>

/*
 * timer_t is the struct k_timer pointer itself. In user mode, stale or foreign handles are
 * rejected by kernel object validation in every syscall; in kernel mode, sys_timer_delete()
 * validates pool membership. There is no module-side per-timer state: all arming, timing,
 * notification, and overrun state lives kernel-side.
 */
static ALWAYS_INLINE struct k_timer *to_timer(timer_t timerid)
{
	return (struct k_timer *)(uintptr_t)timerid;
}

#endif /* ZEPHYR_LIB_POSIX_OPTIONS_TIMERS_TIMERS_INTERNAL_H_ */
