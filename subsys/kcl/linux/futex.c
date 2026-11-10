/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Futexes: a waiter parks on a semaphore keyed by its process and the word's
 * address; a wake releases up to the requested number of matching waiters.
 * Only process-private futexes exist, since every process has its own view
 * of memory.
 */

#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/spinlock.h>
#include <zephyr/sys/clock.h>
#include <zephyr/sys/util.h>

#include "kcl_linux.h"

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1
#define FUTEX_WAIT_BITSET 9
#define FUTEX_WAKE_BITSET 10
#define FUTEX_PRIVATE_FLAG 128
#define FUTEX_CLOCK_REALTIME 256
#define FUTEX_CMD_MASK ~(FUTEX_PRIVATE_FLAG | FUTEX_CLOCK_REALTIME)
#define FUTEX_BITSET_MATCH_ANY 0xffffffffU

struct futex_waiter {
	const struct zkcl_linux_process *proc;
	const uint32_t *addr;
	uint32_t bitset;
	struct k_sem sem;
};

static struct futex_waiter waiters[CONFIG_KCL_LINUX_FUTEX_WAITERS];
static struct k_spinlock lock;

/* the clocks as clock_gettime() reports them */
static int64_t now_ns(bool realtime)
{
	struct timespec ts;

	if (!realtime && IS_ENABLED(CONFIG_TIMER_HAS_64BIT_CYCLE_COUNTER)) {
		return (int64_t)k_cyc_to_ns_floor64(k_cycle_get_64());
	}
	if (sys_clock_gettime(realtime ? SYS_CLOCK_REALTIME : SYS_CLOCK_MONOTONIC, &ts) != 0) {
		return 0;
	}

	return ((int64_t)ts.tv_sec * NSEC_PER_SEC) + ts.tv_nsec;
}

static long futex_wait(const uint32_t *uaddr, uint32_t val, uint32_t bitset,
		       const struct zkcl_linux_timespec *utime, int absolute_clock)
{
	struct zkcl_linux_process *p = zkcl_linux_current();
	struct futex_waiter *w = NULL;
	k_timeout_t timeout = K_FOREVER;
	k_spinlock_key_t key;
	int ret;

	if (utime != NULL) {
		int64_t ns;

		if ((zkcl_linux_user_ok(utime, sizeof(*utime), false) != 0)) {
			return -EFAULT;
		}
		if ((utime->tv_nsec < 0) || (utime->tv_nsec >= NSEC_PER_SEC)) {
			return -EINVAL;
		}
		ns = (utime->tv_sec * NSEC_PER_SEC) + utime->tv_nsec;
		if (absolute_clock >= 0) {
			ns -= now_ns(absolute_clock == 1);
		}
		timeout = (ns <= 0) ? K_NO_WAIT : K_NSEC(ns);
	}

	key = k_spin_lock(&lock);
	if (*uaddr != val) {
		k_spin_unlock(&lock, key);
		return -EAGAIN;
	}
	for (size_t i = 0; i < ARRAY_SIZE(waiters); i++) {
		if (waiters[i].addr == NULL) {
			w = &waiters[i];
			break;
		}
	}
	if (w == NULL) {
		k_spin_unlock(&lock, key);
		return -ENOMEM;
	}
	w->proc = p;
	w->addr = uaddr;
	w->bitset = bitset;
	k_sem_init(&w->sem, 0, 1);
	k_spin_unlock(&lock, key);

	ret = k_sem_take(&w->sem, timeout);

	key = k_spin_lock(&lock);
	w->addr = NULL;
	k_spin_unlock(&lock, key);

	return (ret == 0) ? 0 : -ETIMEDOUT;
}

static long futex_wake(const uint32_t *uaddr, uint32_t nr, uint32_t bitset)
{
	const struct zkcl_linux_process *p = zkcl_linux_current();
	k_spinlock_key_t key = k_spin_lock(&lock);
	long woken = 0;

	for (size_t i = 0; (i < ARRAY_SIZE(waiters)) && (woken < nr); i++) {
		struct futex_waiter *w = &waiters[i];

		if ((w->addr == uaddr) && (w->proc == p) && ((w->bitset & bitset) != 0U)) {
			k_sem_give(&w->sem);
			woken++;
		}
	}
	k_spin_unlock(&lock, key);

	return woken;
}

ZKCL_LINUX_IMPL(futex)(uint32_t *uaddr, int op, uint32_t val,
				     const struct zkcl_linux_timespec *utime, uint32_t *uaddr2,
				     uint32_t val3)
{
	const int cmd = op & FUTEX_CMD_MASK;

	ARG_UNUSED(uaddr2);
	if ((zkcl_linux_current() == NULL) || (((uintptr_t)uaddr & 3) != 0) ||
	    (zkcl_linux_user_ok(uaddr, sizeof(*uaddr), true) != 0)) {
		return -EINVAL;
	}
	switch (cmd) {
	case FUTEX_WAIT:
		return futex_wait(uaddr, val, FUTEX_BITSET_MATCH_ANY, utime, -1);
	case FUTEX_WAIT_BITSET:
		if (val3 == 0U) {
			return -EINVAL;
		}
		return futex_wait(uaddr, val, val3, utime, ((op & FUTEX_CLOCK_REALTIME) != 0) ? 1 : 0);
	case FUTEX_WAKE:
		return futex_wake(uaddr, val, FUTEX_BITSET_MATCH_ANY);
	case FUTEX_WAKE_BITSET:
		if (val3 == 0U) {
			return -EINVAL;
		}
		return futex_wake(uaddr, val, val3);
	default:
		return -ENOSYS;
	}
}
