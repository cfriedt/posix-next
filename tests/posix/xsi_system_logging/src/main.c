/*
 * Copyright (c) 2024, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <syslog.h>
#include <unistd.h>

#if defined(CONFIG_NATIVE_LIBC)
/* the host libc's object-like LOG_ERR cannot coexist with Zephyr's function-like LOG_ERR() */
#undef LOG_ERR
#define POSIX_LOG_ERR 3
#else
#define POSIX_LOG_ERR LOG_ERR
#endif

#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

LOG_MODULE_REGISTER(xsi_system_logging_test);

#define N_PRIOS 8

ZTEST_USER(xsi_system_logging, test_syslog)
{
	int prios[N_PRIOS] = {
		LOG_EMERG,   LOG_ALERT,  LOG_CRIT, POSIX_LOG_ERR,
		LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG,
	};

	openlog("syslog", LOG_PID | LOG_CONS | LOG_NOWAIT, LOG_LOCAL7);
	/*
	 * Enable every priority up to LOG_DEBUG. Spelled out rather than
	 * LOG_MASK(-1): the host libc's LOG_MASK is (1 << (pri)), so -1 is a
	 * negative shift.
	 */
	(void)setlogmask((1 << (LOG_DEBUG + 1)) - 1);

	for (size_t i = 0; i < N_PRIOS; ++i) {
		syslog(i, "syslog priority %d", prios[i]);
	}

	/* Zephyr's function-like LOG_ERR() is usable alongside the <syslog.h> priority */
	LOG_ERR("zephyr log priority %d", POSIX_LOG_ERR);

	closelog();

	/* yield briefly to logging thread */
	usleep(100000);
}

ZTEST_SUITE(xsi_system_logging, NULL, NULL, NULL, NULL, NULL);
