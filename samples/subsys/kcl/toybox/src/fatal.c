/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * A crashing program is not a crashing system: a fatal error in a user-mode
 * thread only ends that thread's process, as a signal would on Linux. Its
 * descriptors close here and now, so that the other end of a pipe sees the
 * end of the stream before the process is reaped. Anything else halts the
 * system as usual.
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/sys/process.h>
#include <zephyr/sys/zvfs.h>

void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *esf)
{
	k_tid_t self = k_current_get();

	ARG_UNUSED(esf);
	if ((reason != K_ERR_KERNEL_PANIC) && ((self->base.user_options & K_USER) != 0U)) {
		printf("process %d: fatal error %u, terminated\n", sys_process_id(k_getpid()),
		       reason);
		for (int fd = 0; fd < CONFIG_ZVFS_OPEN_MAX; fd++) {
			(void)zvfs_close(fd);
		}
		return;
	}
	LOG_PANIC();
	printf("Halting system\n");
	k_fatal_halt(reason);
}
