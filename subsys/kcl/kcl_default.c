/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/* the kernel's hooks when no compatibility layer is enabled */

#include <errno.h>

#include <zephyr/kernel.h>
#include <kernel_internal.h>

int z_kcl_exec_load(struct k_process *proc, k_tid_t leader, const char *path,
		    struct sys_process_start *start)
{
	ARG_UNUSED(proc);
	ARG_UNUSED(leader);
	ARG_UNUSED(path);
	ARG_UNUSED(start);

	return -ENOEXEC;
}

void z_kcl_process_drop(struct k_process *proc)
{
	ARG_UNUSED(proc);
}
