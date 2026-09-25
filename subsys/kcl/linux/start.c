/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * The first code of a Linux process, executed by its leader in user mode:
 * lays out the initial stack the way the Linux kernel's binfmt_elf does and
 * jumps to the executable's entry point.
 */

#include <string.h>

#include <zephyr/kcl/linux/exec.h>
#include <zephyr/kcl/linux/syscalls.h>
#include <zephyr/kernel.h>

#include "kcl_linux.h"

#define AT_NULL 0
#define AT_PHDR 3
#define AT_PHENT 4
#define AT_PHNUM 5
#define AT_PAGESZ 6
#define AT_BASE 7
#define AT_FLAGS 8
#define AT_ENTRY 9
#define AT_UID 11
#define AT_EUID 12
#define AT_GID 13
#define AT_EGID 14
#define AT_PLATFORM 15
#define AT_HWCAP 16
#define AT_CLKTCK 17
#define AT_SECURE 23
#define AT_RANDOM 25
#define AT_EXECFN 31

#define AUXV_ENTRIES 17

#if defined(CONFIG_X86_64)
#define PLATFORM "x86_64"
#elif defined(CONFIG_X86)
#define PLATFORM "i686"
#elif defined(CONFIG_ARM64)
#define PLATFORM "aarch64"
#elif defined(CONFIG_ARM)
#define PLATFORM "v7l"
#elif defined(CONFIG_RISCV)
#define PLATFORM "riscv"
#else
#define PLATFORM "unknown"
#endif

static uintptr_t push_bytes(uintptr_t sp, const void *data, size_t len)
{
	sp -= len;
	memcpy((void *)sp, data, len);

	return sp;
}

static FUNC_NORETURN void jump(uintptr_t sp, uintptr_t entry)
{
#if defined(CONFIG_X86_64)
	/* rdx: no shared-object finalizer to register */
	__asm__ volatile("movq %%rdi, %%rsp\n\t"
			 "xorl %%edx, %%edx\n\t"
			 "jmp *%%rsi\n\t"
			 :
			 : "D"(sp), "S"(entry)
			 : "memory");
#elif defined(CONFIG_X86)
	__asm__ volatile("movl %%edi, %%esp\n\t"
			 "xorl %%edx, %%edx\n\t"
			 "jmp *%%esi\n\t"
			 :
			 : "D"(sp), "S"(entry)
			 : "memory");
#elif defined(CONFIG_ARM64)
	register uintptr_t x0 __asm__("x0") = sp;
	register uintptr_t x1 __asm__("x1") = entry;

	__asm__ volatile("mov sp, x0\n\t"
			 "mov x0, #0\n\t"
			 "br x1\n\t"
			 :
			 : "r"(x0), "r"(x1)
			 : "memory");
#elif defined(CONFIG_ARM)
	register uintptr_t r0 __asm__("r0") = sp;
	register uintptr_t r1 __asm__("r1") = entry;

	__asm__ volatile("mov sp, r0\n\t"
			 "mov r0, #0\n\t"
			 "bx r1\n\t"
			 :
			 : "r"(r0), "r"(r1)
			 : "memory");
#elif defined(CONFIG_RISCV)
	register uintptr_t a0 __asm__("a0") = sp;
	register uintptr_t a1 __asm__("a1") = entry;

	__asm__ volatile("mv sp, a0\n\t"
			 "li a0, 0\n\t"
			 "jr a1\n\t"
			 :
			 : "r"(a0), "r"(a1)
			 : "memory");
#else
#error "no entry sequence for this architecture"
#endif
	CODE_UNREACHABLE;
}

void zkcl_linux_start(void *p1, void *p2, void *p3)
{
	char **argv = p1;
	char **envp = p2;
	int argc = (int)(uintptr_t)p3;
	struct zkcl_linux_process_info info;
	uintptr_t sp;
	uintptr_t *vec;
	uintptr_t random;
	uintptr_t platform;
	uintptr_t execfn;
	size_t envc = 0;
	size_t words;

	if (zkcl_linux_process_info(&info) != 0) {
		zkcl_linux_syscall_exit_group(127);
	}
	while (envp[envc] != NULL) {
		envc++;
	}

	/* strings at the top: execfn, the environment, the arguments */
	sp = info.stack + info.stack_size;
	sp = execfn = push_bytes(sp, argv[0], strlen(argv[0]) + 1);
	for (size_t i = envc; i > 0; i--) {
		sp = push_bytes(sp, envp[i - 1], strlen(envp[i - 1]) + 1);
		envp[i - 1] = (char *)sp;
	}
	for (int i = argc; i > 0; i--) {
		sp = push_bytes(sp, argv[i - 1], strlen(argv[i - 1]) + 1);
		argv[i - 1] = (char *)sp;
	}
	sp = platform = push_bytes(sp, PLATFORM, sizeof(PLATFORM));
	sp = random = push_bytes(sp, info.random, sizeof(info.random));

	/* then the vectors, so that argc lands 16-byte aligned */
	words = 1 + (argc + 1) + (envc + 1) + 2 * (AUXV_ENTRIES + 1);
	sp = ROUND_DOWN(sp - words * sizeof(uintptr_t), 16);
	vec = (uintptr_t *)sp;
	*vec++ = argc;
	for (int i = 0; i < argc; i++) {
		*vec++ = (uintptr_t)argv[i];
	}
	*vec++ = 0;
	for (size_t i = 0; i < envc; i++) {
		*vec++ = (uintptr_t)envp[i];
	}
	*vec++ = 0;

	const uintptr_t auxv[][2] = {
		{AT_PHDR, info.phdr},
		{AT_PHENT, info.phentsize},
		{AT_PHNUM, info.phnum},
		{AT_PAGESZ, ZKCL_LINUX_PAGE_SIZE},
		{AT_BASE, 0},
		{AT_FLAGS, 0},
		{AT_ENTRY, info.entry},
		{AT_UID, 0},
		{AT_EUID, 0},
		{AT_GID, 0},
		{AT_EGID, 0},
		{AT_PLATFORM, platform},
		{AT_HWCAP, 0},
		{AT_CLKTCK, 100},
		{AT_SECURE, 0},
		{AT_RANDOM, random},
		{AT_EXECFN, execfn},
	};
	BUILD_ASSERT(ARRAY_SIZE(auxv) == AUXV_ENTRIES);
	for (size_t i = 0; i < ARRAY_SIZE(auxv); i++) {
		*vec++ = auxv[i][0];
		*vec++ = auxv[i][1];
	}
	*vec++ = AT_NULL;
	*vec++ = 0;

	zkcl_linux_enter();
	jump(sp, info.entry);
}
