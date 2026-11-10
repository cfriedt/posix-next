/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * The user tracing backend's system call hooks: every Linux system call the
 * guest makes is logged as one line, which scripts/linux_syscall_graph.py
 * turns into a graph.
 */

#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/process.h>

#define LINUX_PREFIX "zkcl_linux_syscall_"

void sys_trace_syscall_enter_user(uint32_t syscall_id, const char *syscall_name)
{
	ARG_UNUSED(syscall_id);
	if (strncmp(syscall_name, LINUX_PREFIX, strlen(LINUX_PREFIX)) == 0) {
		printk("linux-syscall: pid=%d %s\n", sys_process_id(k_getpid()),
		       syscall_name + strlen(LINUX_PREFIX));
	}
}

void sys_trace_syscall_exit_user(uint32_t syscall_id, const char *syscall_name)
{
	ARG_UNUSED(syscall_id);
	ARG_UNUSED(syscall_name);
}
