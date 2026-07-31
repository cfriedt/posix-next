/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "message_passing_tests.h"

static void mq_timedsend_immediate(void)
{
	char buf[MQ_MSG_SIZE];
	struct timespec ts = mq_test_deadline(MQ_TIMEOUT_MS);
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	/* space is available, so the deadline never comes into play */
	zassert_ok(mq_timedsend(mqd, "timed", 6, 3, &ts));
	zassert_equal(mq_receive(mqd, buf, sizeof(buf), NULL), 6);
	zassert_mem_equal(buf, "timed", 6);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_timedsend_times_out(void)
{
	struct timespec ts;
	struct timespec before;
	struct timespec after;
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);
	mq_test_fill(mqd, 0);

	zassert_ok(clock_gettime(CLOCK_REALTIME, &before));
	ts = mq_test_deadline(MQ_TIMEOUT_MS);
	zassert_equal(mq_timedsend(mqd, "late", 5, 0, &ts), -1);
	zassert_equal(errno, ETIMEDOUT);
	zassert_ok(clock_gettime(CLOCK_REALTIME, &after));

	/* the call waited for the deadline rather than returning at once */
	int64_t elapsed_ms = (int64_t)(after.tv_sec - before.tv_sec) * MSEC_PER_SEC +
			     (after.tv_nsec - before.tv_nsec) / NSEC_PER_MSEC;

	zassert_true(elapsed_ms >= MQ_TIMEOUT_MS, "returned early after %lld ms", elapsed_ms);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_timedsend_expired_deadline(void)
{
	struct timespec ts = mq_test_deadline(-MQ_TIMEOUT_MS);
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);
	mq_test_fill(mqd, 0);

	/* a deadline already in the past does not wait */
	zassert_equal(mq_timedsend(mqd, "past", 5, 0, &ts), -1);
	zassert_equal(errno, ETIMEDOUT);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_timedsend_errors(void)
{
	struct timespec ts = mq_test_deadline(MQ_TIMEOUT_MS);
	struct timespec bad = {.tv_sec = 0, .tv_nsec = NSEC_PER_SEC};
	char big[MQ_MSG_SIZE + 1] = {0};
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	/*
	 * abstime is only required to be validated when the call would block,
	 * so the malformed deadline is checked against a full queue.
	 */
	mq_test_fill(mqd, 0);
	zassert_equal(mq_timedsend(mqd, "msg", 4, 0, &bad), -1);
	zassert_equal(errno, EINVAL);

	zassert_equal(mq_timedsend(mqd, big, sizeof(big), 0, &ts), -1);
	zassert_equal(errno, EMSGSIZE);

	zassert_equal(mq_timedsend((mqd_t)-1, "msg", 4, 0, &ts), -1);
	zassert_equal(errno, EBADF);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

ZTEST_USER(posix_message_passing, test_mq_timedsend)
{
	mq_timedsend_immediate();
	mq_test_section_reset();
	mq_timedsend_times_out();
	mq_test_section_reset();
	mq_timedsend_expired_deadline();
	mq_test_section_reset();
	mq_timedsend_errors();
	mq_test_section_reset();
}
