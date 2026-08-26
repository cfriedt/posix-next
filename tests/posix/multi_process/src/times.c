/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

static K_THREAD_STACK_DEFINE(burner_stack, 1024);
static struct k_thread burner_thread;

static void burner_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	/* burn a measurable slice of CPU, then exit; k_busy_wait() also
	 * advances simulated time, which a compute loop does not
	 */
	k_busy_wait(20U * USEC_PER_MSEC);
	k_exit(0);
}

static void spawn_burner_check(void)
{
	struct tms before = {0};
	struct tms after = {0};
	struct sys_clone_args args = {
		.entry = burner_entry,
		.thread = &burner_thread,
		.stack = burner_stack,
		.stack_size = 1024,
		.prio = k_thread_priority_get(k_current_get()),
	};
	k_pid_t child = NULL;
	pid_t pid;

	(void)times(&before);
	zassert_ok(sys_clone(&args, &child));
	pid = (pid_t)sys_process_id(child);
	zassert_true(pid > 0);
	zassert_equal(waitpid(pid, NULL, 0), pid);
	(void)times(&after);

	zassert_true(after.tms_cutime > before.tms_cutime,
		     "cutime did not grow after reaping a busy child: %ld -> %ld",
		     (long)before.tms_cutime, (long)after.tms_cutime);
}

ZTEST(posix_multi_process, test_times)
{
	static const struct {
		const char *name;
		size_t offset;
	} fields[] = {
		{
			.name = "utime",
			.offset = offsetof(struct tms, tms_utime),
		},
		{
			.name = "stime",
			.offset = offsetof(struct tms, tms_stime),
		},
		{
			.name = "cutime",
			.offset = offsetof(struct tms, tms_cutime),
		},
		{
			.name = "cstime",
			.offset = offsetof(struct tms, tms_cstime),
		},
	};
	struct tms test_tms[2] = {};
	clock_t rtime[2];

	rtime[0] = times(&test_tms[0]);
	k_msleep(MSEC_PER_SEC);
	rtime[1] = times(&test_tms[1]);

	zexpect_not_equal(rtime[0], -1);
	zexpect_not_equal(rtime[1], -1);

	printk("t0: rtime: %ld utime: %ld stime: %ld cutime: %ld cstime: %ld\n", rtime[0],
	       test_tms[0].tms_utime, test_tms[0].tms_stime, test_tms[0].tms_cutime,
	       test_tms[0].tms_cstime);
	printk("t1: rtime: %ld utime: %ld stime: %ld cutime: %ld cstime: %ld\n", rtime[1],
	       test_tms[1].tms_utime, test_tms[1].tms_stime, test_tms[1].tms_cutime,
	       test_tms[1].tms_cstime);

	ARRAY_FOR_EACH(fields, i) {
		const char *name = fields[i].name;
		size_t offset = fields[i].offset;

		clock_t t0 = *(clock_t *)((uint8_t *)&test_tms[0] + offset);
		clock_t t1 = *(clock_t *)((uint8_t *)&test_tms[1] + offset);

		zexpect_true(t1 >= t0, "time moved backward for tms_%s: t0: %ld t1: %ld", name, t0,
			     t1);
	}

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC) && !k_is_user_context()) {
		/* a reaped CPU-burning child must add to the parent's cutime */
		spawn_burner_check();
	}
}
