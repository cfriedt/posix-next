/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "message_passing_tests.h"

static void mq_open_create_and_reopen(void)
{
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1, "mq_open() failed: %d", errno);

	/* an existing queue is reopened without O_CREAT and without attributes */
	mqd_t mqd2 = mq_open(MQ_NAME, O_RDWR);

	zassert_not_equal(mqd2, (mqd_t)-1, "reopen failed: %d", errno);
	zassert_not_equal(mqd, mqd2, "each open returns a distinct descriptor");

	zassert_ok(mq_close(mqd2));
	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_open_persists_until_unlinked(void)
{
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);
	zassert_ok(mq_close(mqd));

	/* closing every descriptor does not destroy the queue */
	mqd = mq_open(MQ_NAME, O_RDWR);
	zassert_not_equal(mqd, (mqd_t)-1, "queue did not outlive its descriptors: %d", errno);
	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));

	/* after unlinking it is gone */
	zassert_equal(mq_open(MQ_NAME, O_RDWR), (mqd_t)-1);
	zassert_equal(errno, ENOENT);
}

static void mq_open_access_modes(void)
{
	char buf[MQ_MSG_SIZE];
	mqd_t rdwr = mq_test_open(0);
	mqd_t rdonly;
	mqd_t wronly;

	zassert_not_equal(rdwr, (mqd_t)-1);
	rdonly = mq_open(MQ_NAME, O_RDONLY);
	wronly = mq_open(MQ_NAME, O_WRONLY);
	zassert_not_equal(rdonly, (mqd_t)-1);
	zassert_not_equal(wronly, (mqd_t)-1);

	/* the access mode is per descriptor */
	zassert_ok(mq_send(wronly, "hello", 6, 0));
	zassert_equal(mq_send(rdonly, "hello", 6, 0), -1);
	zassert_equal(errno, EBADF);

	zassert_equal(mq_receive(rdonly, buf, sizeof(buf), NULL), 6);
	zassert_equal(mq_receive(wronly, buf, sizeof(buf), NULL), -1);
	zassert_equal(errno, EBADF);

	zassert_ok(mq_close(wronly));
	zassert_ok(mq_close(rdonly));
	zassert_ok(mq_close(rdwr));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_open_excl(void)
{
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	/* O_CREAT | O_EXCL on an existing queue */
	struct mq_attr attr = mq_test_attr;

	zassert_equal(mq_open(MQ_NAME, O_RDWR | O_CREAT | O_EXCL, 0600, &attr), (mqd_t)-1);
	zassert_equal(errno, EEXIST);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));

	/* and on a fresh name it succeeds */
	mqd = mq_open(MQ_NAME, O_RDWR | O_CREAT | O_EXCL, 0600, &attr);
	zassert_not_equal(mqd, (mqd_t)-1, "O_EXCL create failed: %d", errno);
	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_open_errors(void)
{
	struct mq_attr attr;

	/* opening a queue that does not exist, without O_CREAT */
	zassert_equal(mq_open(MQ_NAME, O_RDWR), (mqd_t)-1);
	zassert_equal(errno, ENOENT);

	/* creating without attributes, or with degenerate geometry */
	attr = mq_test_attr;
	attr.mq_msgsize = 0;
	zassert_equal(mq_open(MQ_NAME, O_RDWR | O_CREAT, 0600, &attr), (mqd_t)-1);
	zassert_equal(errno, EINVAL);

	attr = mq_test_attr;
	attr.mq_maxmsg = 0;
	zassert_equal(mq_open(MQ_NAME, O_RDWR | O_CREAT, 0600, &attr), (mqd_t)-1);
	zassert_equal(errno, EINVAL);

	attr = mq_test_attr;
	attr.mq_msgsize = -1;
	zassert_equal(mq_open(MQ_NAME, O_RDWR | O_CREAT, 0600, &attr), (mqd_t)-1);
	zassert_equal(errno, EINVAL);
}

ZTEST_USER(posix_message_passing, test_mq_open)
{
	mq_open_create_and_reopen();
	mq_test_section_reset();
	mq_open_persists_until_unlinked();
	mq_test_section_reset();
	mq_open_access_modes();
	mq_test_section_reset();
	mq_open_excl();
	mq_test_section_reset();
	mq_open_errors();
	mq_test_section_reset();
}
