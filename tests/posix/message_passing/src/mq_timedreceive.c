/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "message_passing_tests.h"

static void mq_timedreceive_immediate(void)
{
	char buf[MQ_MSG_SIZE];
	unsigned int prio;
	struct timespec ts = mq_test_deadline(MQ_TIMEOUT_MS);
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);
	zassert_ok(mq_send(mqd, "ready", 6, 4));

	/* a message is available, so the deadline never comes into play */
	zassert_equal(mq_timedreceive(mqd, buf, sizeof(buf), &prio, &ts), 6);
	zassert_equal(prio, 4);
	zassert_mem_equal(buf, "ready", 6);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_timedreceive_times_out(void)
{
	char buf[MQ_MSG_SIZE];
	struct timespec ts;
	struct timespec before;
	struct timespec after;
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	zassert_ok(clock_gettime(CLOCK_REALTIME, &before));
	ts = mq_test_deadline(MQ_TIMEOUT_MS);
	zassert_equal(mq_timedreceive(mqd, buf, sizeof(buf), NULL, &ts), -1);
	zassert_equal(errno, ETIMEDOUT);
	zassert_ok(clock_gettime(CLOCK_REALTIME, &after));

	int64_t elapsed_ms = (int64_t)(after.tv_sec - before.tv_sec) * MSEC_PER_SEC +
			     (after.tv_nsec - before.tv_nsec) / NSEC_PER_MSEC;

	zassert_true(elapsed_ms >= MQ_TIMEOUT_MS, "returned early after %lld ms", elapsed_ms);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_timedreceive_expired_deadline(void)
{
	char buf[MQ_MSG_SIZE];
	struct timespec ts = mq_test_deadline(-MQ_TIMEOUT_MS);
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	zassert_equal(mq_timedreceive(mqd, buf, sizeof(buf), NULL, &ts), -1);
	zassert_equal(errno, ETIMEDOUT);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_timedreceive_errors(void)
{
	char buf[MQ_MSG_SIZE];
	char small[MQ_MSG_SIZE - 1];
	struct timespec ts = mq_test_deadline(MQ_TIMEOUT_MS);
	struct timespec bad = {.tv_sec = 0, .tv_nsec = NSEC_PER_SEC};
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	/*
	 * abstime is only required to be validated when the call would block,
	 * so the malformed deadline is checked against an empty queue.
	 */
	zassert_equal(mq_timedreceive(mqd, buf, sizeof(buf), NULL, &bad), -1);
	zassert_equal(errno, EINVAL);

	zassert_ok(mq_send(mqd, "msg", 4, 0));

	zassert_equal(mq_timedreceive(mqd, small, sizeof(small), NULL, &ts), -1);
	zassert_equal(errno, EMSGSIZE);

	zassert_equal(mq_timedreceive((mqd_t)-1, buf, sizeof(buf), NULL, &ts), -1);
	zassert_equal(errno, EBADF);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

ZTEST_USER(posix_message_passing, test_mq_timedreceive)
{
	mq_timedreceive_immediate();
	mq_test_section_reset();
	mq_timedreceive_times_out();
	mq_test_section_reset();
	mq_timedreceive_expired_deadline();
	mq_test_section_reset();
	mq_timedreceive_errors();
	mq_test_section_reset();
}
