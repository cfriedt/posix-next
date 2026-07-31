/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_MESSAGE_PASSING_TESTS_H_
#define POSIX_MESSAGE_PASSING_TESTS_H_

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <mqueue.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <zephyr/sys/timeutil.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include <signal.h>

/**
 * @brief Accept one instance of @p signo, waiting up to @p timeout_ms.
 *
 * On @kconfig{CONFIG_NATIVE_LIBC} the signal is a real host signal: a blocking
 * host sigtimedwait() would stall native_sim, so deliveries are captured by a
 * handler and consumed from a counter instead.
 *
 * @return the signal number, or -1 if none arrived in time.
 */
int mq_accept_sig(int signo, siginfo_t *info, int timeout_ms);

/** @brief Arrange for @p signo to be accepted by mq_accept_sig() rather than delivered. */
void mq_arm_sig(int signo);

/** @brief Discard any pending instances of @p signo. */
static inline void mq_drain_sig(int signo)
{
	while (mq_accept_sig(signo, NULL, 0) >= 0) {
	}
}

#ifndef MQ_PRIO_MAX
/* not every libc's <limits.h> exposes the POSIX runtime invariant values */
#define MQ_PRIO_MAX CONFIG_POSIX_MQ_PRIO_MAX
#endif

/* POSIX requires a portable queue name to begin with a slash */
#define MQ_NAME  "/mq_test"
#define MQ_NAME2 "/mq_test2"

#define MQ_MSG_SIZE 16
/* at most the Linux default fs.mqueue.msg_max, so linux_compat can create it */
#define MQ_MAX_MSGS 4

#define MQ_TIMEOUT_MS 100

static const struct mq_attr mq_test_attr = {
	.mq_msgsize = MQ_MSG_SIZE,
	.mq_maxmsg = MQ_MAX_MSGS,
};

/** @brief Create (or reopen) the test queue read-write. */
static inline mqd_t mq_test_open(int extra_oflags)
{
	struct mq_attr attr = mq_test_attr;

	return mq_open(MQ_NAME, O_RDWR | O_CREAT | extra_oflags, 0600, &attr);
}

/** @brief Absolute deadline @p ms milliseconds from now, for the timed calls. */
static inline struct timespec mq_test_deadline(long ms)
{
	struct timespec ts = {0};

	/* the per-second constants are unsigned: keep the arithmetic signed */
	struct timespec delta = {
		.tv_sec = ms / (long)MSEC_PER_SEC,
		.tv_nsec = (ms % (long)MSEC_PER_SEC) * (long)NSEC_PER_MSEC,
	};

	(void)clock_gettime(CLOCK_REALTIME, &ts);
	/* @p ms may be negative, for a deadline that has already passed */
	(void)timespec_normalize(&delta);
	(void)timespec_add(&ts, &delta);

	return ts;
}

/**
 * @brief Return the queue to a known state between sections.
 *
 * Unlinking removes the name immediately, so the next section's mq_open()
 * creates a fresh queue even if a descriptor from this one is still open.
 */
static inline void mq_test_section_reset(void)
{
	(void)mq_unlink(MQ_NAME);
	(void)mq_unlink(MQ_NAME2);
}

/** @brief Fill the queue to capacity at priority @p prio. */
static inline void mq_test_fill(mqd_t mqd, unsigned int prio)
{
	static const char msg[MQ_MSG_SIZE] = "fill";

	for (int i = 0; i < MQ_MAX_MSGS; i++) {
		zassert_ok(mq_send(mqd, msg, sizeof(msg), prio));
	}
}

#endif /* POSIX_MESSAGE_PASSING_TESTS_H_ */
