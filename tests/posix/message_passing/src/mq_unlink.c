/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "message_passing_tests.h"

static void mq_unlink_frees_name_immediately(void)
{
	char buf[MQ_MSG_SIZE];
	mqd_t old = mq_test_open(0);
	mqd_t fresh;

	zassert_not_equal(old, (mqd_t)-1);
	zassert_ok(mq_send(old, "old", 4, 0));
	zassert_ok(mq_unlink(MQ_NAME));

	/* the open descriptor keeps working on the unlinked queue */
	zassert_equal(mq_receive(old, buf, sizeof(buf), NULL), 4);
	zassert_mem_equal(buf, "old", 4);

	/* while the name is immediately reusable for a new, distinct queue */
	fresh = mq_test_open(0);
	zassert_not_equal(fresh, (mqd_t)-1, "name not reusable after unlink: %d", errno);
	zassert_ok(mq_send(fresh, "new", 4, 0));

	/* the two queues are independent: the message went to the new one only */
	struct timespec ts = mq_test_deadline(MQ_TIMEOUT_MS);

	zassert_equal(mq_timedreceive(old, buf, sizeof(buf), NULL, &ts), -1);
	zassert_equal(errno, ETIMEDOUT, "descriptors alias the same queue");

	zassert_equal(mq_receive(fresh, buf, sizeof(buf), NULL), 4);
	zassert_mem_equal(buf, "new", 4);

	zassert_ok(mq_close(fresh));
	zassert_ok(mq_close(old));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_unlink_multiple(void)
{
	struct mq_attr attr = mq_test_attr;
	mqd_t a = mq_test_open(0);
	mqd_t b = mq_open(MQ_NAME2, O_RDWR | O_CREAT, 0600, &attr);

	zassert_not_equal(a, (mqd_t)-1);
	zassert_not_equal(b, (mqd_t)-1);

	zassert_ok(mq_unlink(MQ_NAME));
	zassert_ok(mq_unlink(MQ_NAME2));

	zassert_ok(mq_close(a));
	zassert_ok(mq_close(b));
}

static void mq_unlink_errors(void)
{
	zassert_equal(mq_unlink(MQ_NAME), -1);
	zassert_equal(errno, ENOENT);

	zassert_equal(mq_unlink("/no_such_queue"), -1);
	zassert_equal(errno, ENOENT);
}

ZTEST_USER(posix_message_passing, test_mq_unlink)
{
	mq_unlink_frees_name_immediately();
	mq_test_section_reset();
	mq_unlink_multiple();
	mq_test_section_reset();
	mq_unlink_errors();
	mq_test_section_reset();
}
