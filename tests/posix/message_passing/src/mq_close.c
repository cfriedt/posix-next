/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "message_passing_tests.h"

static void mq_close_releases_descriptor(void)
{
	char buf[MQ_MSG_SIZE];
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);
	zassert_ok(mq_close(mqd));

	/* the descriptor is no longer usable */
	zassert_equal(mq_receive(mqd, buf, sizeof(buf), NULL), -1);
	zassert_equal(errno, EBADF);

	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_close_other_descriptors_survive(void)
{
	char buf[MQ_MSG_SIZE];
	mqd_t a = mq_test_open(0);
	mqd_t b = mq_open(MQ_NAME, O_RDWR);

	zassert_not_equal(a, (mqd_t)-1);
	zassert_not_equal(b, (mqd_t)-1);

	zassert_ok(mq_send(a, "shared", 7, 0));
	zassert_ok(mq_close(a));

	/* messages sent through a closed descriptor remain queued */
	zassert_equal(mq_receive(b, buf, sizeof(buf), NULL), 7);
	zassert_mem_equal(buf, "shared", 7);

	zassert_ok(mq_close(b));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_close_errors(void)
{
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);
	zassert_ok(mq_close(mqd));

	/* closing twice */
	zassert_equal(mq_close(mqd), -1);
	zassert_equal(errno, EBADF);

	/* closing a descriptor that was never open */
	zassert_equal(mq_close((mqd_t)-1), -1);
	zassert_equal(errno, EBADF);

	zassert_ok(mq_unlink(MQ_NAME));
}

ZTEST_USER(posix_message_passing, test_mq_close)
{
	mq_close_releases_descriptor();
	mq_test_section_reset();
	mq_close_other_descriptors_survive();
	mq_test_section_reset();
	mq_close_errors();
	mq_test_section_reset();
}
