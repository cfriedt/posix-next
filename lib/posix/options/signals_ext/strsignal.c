/*
 * Copyright (c) 2023 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef CONFIG_POSIX_SIGNAL_STRING_DESC_DISABLE
#include "posix/strsignal_table.h"
#endif

#include <errno.h>
#include <signal.h>
#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#if defined(CONFIG_POSIX_REALTIME_SIGNALS)
#ifndef _POSIX_REALTIME_SIGNALS
#define _POSIX_REALTIME_SIGNALS 200809L
#endif
#include <zephyr/posix/sys/select.h>
#endif

static inline bool signo_valid(int signo)
{
	if (IS_ENABLED(CONFIG_POSIX_REALTIME_SIGNALS)) {
		return (signo > 0) && (signo <= SIGNAL_SET_SIZE);
	}

	return (signo > 0) && (signo <= SIGRTMAX);
}

static inline bool signo_is_rt(int signo)
{
	return ((signo >= SIGRTMIN) && (signo <= SIGRTMAX));
}

char *strsignal(int signum)
{
	static char buf[sizeof("RT signal " STRINGIFY(UINT_MAX))];

	if (!signo_valid(signum)) {
		errno = EINVAL;
		return "Invalid signal";
	}

	if (signo_is_rt(signum)) {
		snprintf(buf, sizeof(buf), "RT signal %u", (unsigned int)(signum - SIGRTMIN));
		return buf;
	}

#ifndef CONFIG_POSIX_SIGNAL_STRING_DESC_DISABLE
	if (strsignal_list[signum] != NULL) {
		return (char *)strsignal_list[signum];
	}
#endif

	snprintf(buf, sizeof(buf), "Signal %d", signum);

	return buf;
}
