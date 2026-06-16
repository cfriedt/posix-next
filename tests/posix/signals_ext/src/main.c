/*
 * Copyright (c) 2023 Meta
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

/* max length for the purposes of this test */
#define STRSIGNAL_MAX_LEN 32

static bool strsignal_is_unknown(const char *actual)
{
	return (strstr(actual, "Unknown") != NULL) || (strstr(actual, "Invalid") != NULL);
}

ZTEST(posix_signals_ext, test_strsignal)
{
	char *actual;
	/* Using -INT_MAX here because compiler resolves INT_MIN to (-2147483647 - 1) */
	char buf[sizeof("Real-time signal -" STRINGIFY(INT_MAX))] = {0};

	actual = strsignal(-1);
	zexpect_true(strsignal_is_unknown(actual), "actual: '%s'", actual);
	actual = strsignal(0);
	zexpect_true(strsignal_is_unknown(actual), "actual: '%s'", actual);
	actual = strsignal(4242);
	zexpect_true(strsignal_is_unknown(actual), "actual: '%s'", actual);

	static const char *const rts[] = {"Real-time signal", "RT signal"};

	ARRAY_FOR_EACH(rts, i) {
		snprintf(buf, sizeof(buf), "%s %d", rts[i], SIGRTMIN - SIGRTMIN);
		actual = strsignal(SIGRTMIN);

		if (strncmp(rts[i], actual, strlen(rts[i])) != 0) {
			if (i == ARRAY_SIZE(rts) - 1) {
				zexpect_true(false, "Unrecognized real-time prefix: '%s'", actual);
				break;
			}
			continue;
		}

		snprintf(buf, sizeof(buf), "%s %d", rts[i], SIGRTMAX - SIGRTMIN);
		actual = strsignal(SIGRTMAX);
		zexpect_str_equal(actual, buf, "actual: '%s', expected: '%s'", actual, buf);
		break;
	}


	if (IS_ENABLED(CONFIG_POSIX_SIGNAL_STRING_DESC_DISABLE)) {
		snprintf(buf, sizeof(buf), "Signal %d", SIGHUP);
		zexpect_mem_equal(strsignal(SIGHUP), buf, strlen(buf));
		snprintf(buf, sizeof(buf), "Signal %d", SIGSYS);
		zexpect_mem_equal(strsignal(SIGSYS), buf, strlen(buf));
		return;
	}

#define decl_strenum(n) \
	{ \
		.num = (n), .str = #n \
	}

	static const struct strenum {
		int num;
		const char *str;
	} sigs[] = {
		decl_strenum(SIGHUP),
		decl_strenum(SIGINT),
		decl_strenum(SIGQUIT),
		decl_strenum(SIGILL),
		decl_strenum(SIGTRAP),
		decl_strenum(SIGABRT),
		decl_strenum(SIGBUS),
		decl_strenum(SIGFPE),
		decl_strenum(SIGKILL),
		decl_strenum(SIGSEGV),
		decl_strenum(SIGPIPE),
		decl_strenum(SIGALRM),
		decl_strenum(SIGTERM),
		decl_strenum(SIGCHLD),
		decl_strenum(SIGCONT),
		decl_strenum(SIGSTOP),
		decl_strenum(SIGTSTP),
		decl_strenum(SIGTTIN),
		decl_strenum(SIGTTOU),
		decl_strenum(SIGURG),
		decl_strenum(SIGXCPU),
		decl_strenum(SIGXFSZ),
		decl_strenum(SIGVTALRM),
		decl_strenum(SIGPROF),
		decl_strenum(SIGPOLL),
		decl_strenum(SIGSYS),
	};
	char strs[ARRAY_SIZE(sigs)][STRSIGNAL_MAX_LEN];

	ARRAY_FOR_EACH(sigs, i) {
		actual = strsignal(sigs[i].num);
		strncpy(strs[i], actual, sizeof(strs[i]));

		/* check that the strsig() is neither NULL nor empty */
		zexpect_not_null(actual, "strsignal(%s [%d]): is NULL",
				 sigs[i].str, (int)sigs[i].num);
		zexpect_true(strlen(actual) > 0, "strsignal(%s [%d]): is empty",
			     sigs[i].str, (int)sigs[i].num);
	}

	/* check that each signal has a unique string */
	ARRAY_FOR_EACH(sigs, i) {
		ARRAY_FOR_EACH(sigs, j) {
			if (i == j) {
				continue;
			}

			zexpect_not_equal(strncmp(strs[i], strs[j], STRSIGNAL_MAX_LEN), 0,
					  "strsignal(%s [%d]) '%s' == strsignal(%s [%d]) '%s'",
					  sigs[i].str, (int)sigs[i].num, strs[i],
					  sigs[j].str, (int)sigs[j].num, strs[j]);
		}
	}
}

ZTEST_SUITE(posix_signals_ext, NULL, NULL, NULL, NULL, NULL);
