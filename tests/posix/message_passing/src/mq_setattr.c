/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "message_passing_tests.h"

static void mq_setattr_toggles_nonblock(void)
{
	char buf[MQ_MSG_SIZE];
	struct mq_attr attr = {0};
	struct mq_attr out;
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	attr.mq_flags = O_NONBLOCK;
	zassert_ok(mq_setattr(mqd, &attr, NULL));
	zassert_ok(mq_getattr(mqd, &out));
	zassert_equal(out.mq_flags, O_NONBLOCK);

	/* the new flag takes effect on subsequent operations */
	zassert_equal(mq_receive(mqd, buf, sizeof(buf), NULL), -1);
	zassert_equal(errno, EAGAIN);

	attr.mq_flags = 0;
	zassert_ok(mq_setattr(mqd, &attr, NULL));
	zassert_ok(mq_getattr(mqd, &out));
	zassert_equal(out.mq_flags, 0);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_setattr_reports_old_attributes(void)
{
	struct mq_attr attr = {.mq_flags = O_NONBLOCK};
	struct mq_attr old;
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);
	zassert_ok(mq_send(mqd, "one", 4, 0));

	zassert_ok(mq_setattr(mqd, &attr, &old));

	/* omqstat describes the queue as it was before the call */
	zassert_equal(old.mq_flags, 0);
	zassert_equal(old.mq_msgsize, MQ_MSG_SIZE);
	zassert_equal(old.mq_maxmsg, MQ_MAX_MSGS);
	zassert_equal(old.mq_curmsgs, 1);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_setattr_errors(void)
{
	struct mq_attr attr = {0};
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	/* only O_NONBLOCK may be set */
	attr.mq_flags = O_CREAT;
	zassert_equal(mq_setattr(mqd, &attr, NULL), -1);
	zassert_equal(errno, EINVAL);

	attr.mq_flags = 0;
	zassert_equal(mq_setattr((mqd_t)-1, &attr, NULL), -1);
	zassert_equal(errno, EBADF);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

ZTEST_USER(posix_message_passing, test_mq_setattr)
{
	mq_setattr_toggles_nonblock();
	mq_test_section_reset();
	mq_setattr_reports_old_attributes();
	mq_test_section_reset();
	mq_setattr_errors();
	mq_test_section_reset();
}
