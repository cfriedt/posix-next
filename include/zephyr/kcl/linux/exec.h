/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Linux executables as processes
 *
 * @defgroup kcl_linux Linux kernel compatibility layer
 * @ingroup os_services
 * @{
 */

#ifndef ZEPHYR_INCLUDE_ZEPHYR_KCL_LINUX_EXEC_H_
#define ZEPHYR_INCLUDE_ZEPHYR_KCL_LINUX_EXEC_H_

#include <stddef.h>
#include <stdint.h>

#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief What the leader of a Linux process needs to enter its executable
 *
 * Filled in by zkcl_linux_process_info(); the fields feed the auxiliary vector
 * of the initial stack.
 */
struct zkcl_linux_process_info {
	/** Entry point, relocated */
	uintptr_t entry;
	/** Program header table, as loaded */
	uintptr_t phdr;
	/** Number of program headers */
	uint16_t phnum;
	/** Size of one program header */
	uint16_t phentsize;
	/** Lowest address of the stack */
	uintptr_t stack;
	/** Size of the stack */
	size_t stack_size;
	/** Random bytes for the executable's canaries */
	uint8_t random[16];
};

/**
 * @brief Describe the calling process's Linux executable
 *
 * @param info where to write the description
 * @retval 0 on success
 * @retval -ENOENT if the calling process does not run a Linux executable
 */
__syscall int zkcl_linux_process_info(struct zkcl_linux_process_info *info);

/**
 * @brief Switch the calling thread to the Linux system call convention
 *
 * The last thing the layer's start code does before jumping into the
 * executable: from then on the thread's traps are Linux system calls.
 */
__syscall void zkcl_linux_enter(void);

#ifdef __cplusplus
}
#endif

/** @} */

#include <zephyr/syscalls/exec.h>

#endif /* ZEPHYR_INCLUDE_ZEPHYR_KCL_LINUX_EXEC_H_ */
