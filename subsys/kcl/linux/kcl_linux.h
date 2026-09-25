/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_SUBSYS_KCL_LINUX_KCL_LINUX_H_
#define ZEPHYR_SUBSYS_KCL_LINUX_KCL_LINUX_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/zvfs_fs.h>
#include <zephyr/kcl/linux/syscalls.h>
#include <zephyr/kcl/linux/types.h>

#ifdef CONFIG_MMU
#define ZKCL_LINUX_PAGE_SIZE CONFIG_MMU_PAGE_SIZE
#else
#define ZKCL_LINUX_PAGE_SIZE 4096
#endif

/*
 * The definition of a system call's implementation: the dispatched symbol when
 * the call is on, an unreferenced static otherwise - compiled either way, and
 * dropped from the link when off, where the generated default answers ENOSYS.
 */
#define ZKCL_LINUX_IMPL(name)                                                                      \
	COND_CODE_1(ZKCL_LINUX_SYSCALL_##name, (long z_impl_zkcl_linux_syscall_##name),           \
		    (static long __unused zkcl_linux_off_##name))

#define ZKCL_LINUX_MMAP_MAX 8
#define ZKCL_LINUX_SOCK_MAX 8
#define ZKCL_LINUX_PARTS_MAX 8

/* the terminal attributes as the kernel keeps them (TCGETS) */
struct zkcl_linux_termios {
	uint32_t c_iflag;
	uint32_t c_oflag;
	uint32_t c_cflag;
	uint32_t c_lflag;
	uint8_t c_line;
	uint8_t c_cc[19];
};

/* what the layer remembers about a socket beyond what the stack keeps */
struct zkcl_linux_sock {
	int fd;			/* -1: free */
	uint8_t family;		/* NET_AF_INET or NET_AF_INET6 */
	bool icmp_dgram;	/* a Linux "ping socket", emulated over a raw ICMP socket */
	bool recvttl;		/* IP_RECVTTL / IPV6_RECVHOPLIMIT asked for */
	uint16_t echo_id;	/* the id of the last echo request sent */
	uint8_t peer[32];	/* where the last echo request went (struct net_sockaddr_storage) */
};

/* a mapping made by mmap(), one partition each */
struct zkcl_linux_mapping {
	struct k_mem_partition part;
	bool owned;		/* allocated by this process, not inherited by fork() */
	/* a shared writable file mapping: written back through fd, the layer's own duplicate */
	int fd;			/* -1: nothing to write back */
	const void *file;	/* the description fd named when the mapping was made */
	unsigned long off;
	size_t file_len;	/* bytes the file supplied */
};

/* the loaded segments and the brk area, shared by a process and its forks */
struct zkcl_linux_image {
	void *mem;
	size_t size;
	atomic_t refs;
};

/* the per-process state of a Linux executable, hanging off k_process.compat */
struct zkcl_linux_process {
	struct zkcl_linux_image *image;
	void *stack;
	size_t stack_size;
	bool stack_owned;	/* a fork() child runs on the parent's copied frames */
	uintptr_t base;		/* load bias: an address is base + p_vaddr */
	uintptr_t entry;
	uintptr_t phdr;		/* the program headers, as loaded */
	uint16_t phnum;
	uint16_t phentsize;
	uintptr_t brk_start;
	uintptr_t brk;
	uintptr_t brk_end;
	uintptr_t clear_child_tid;
	uint8_t random[16];
	/* the image's and the stack's partitions, to take out of the domain at exec */
	struct k_mem_partition parts[ZKCL_LINUX_PARTS_MAX];
	uint8_t nparts;
	struct zkcl_linux_termios termios;
	struct zkcl_linux_mapping maps[ZKCL_LINUX_MMAP_MAX];
	/* a directory entry read ahead of a getdents64() buffer that could not take it */
	int dirent_fd;
	bool dirent_pending;
	struct zvfs_dirent dirent;
	struct zkcl_linux_sock socks[ZKCL_LINUX_SOCK_MAX];
};

#ifdef CONFIG_NET_SOCKETS
/* the socket state of a descriptor, or NULL */
struct zkcl_linux_sock *zkcl_linux_sock_find(int fd);

/* forget a descriptor's socket state, when it is closed or replaced */
void zkcl_linux_sock_forget(int fd);
#else
static inline struct zkcl_linux_sock *zkcl_linux_sock_find(int fd)
{
	ARG_UNUSED(fd);
	return NULL;
}

static inline void zkcl_linux_sock_forget(int fd)
{
	ARG_UNUSED(fd);
}
#endif /* CONFIG_NET_SOCKETS */

static inline struct zkcl_linux_process *zkcl_linux_current(void)
{
	struct k_process *proc = _current->process;

	return (proc != NULL) ? proc->compat : NULL;
}

/* the Linux errno number for the C library's */
int zkcl_linux_errno(int err);

/* a system call result as the executable expects it: errors renumbered */
long zkcl_linux_result(long ret);

/* 0 when the calling thread may access [ptr, ptr + size), else -EFAULT */
int zkcl_linux_user_ok(const void *ptr, size_t size, bool write);

/* copy a NUL-terminated string from the caller; -EFAULT or -ENAMETOOLONG */
int zkcl_linux_user_string(char *dst, const char *src, size_t size);

/* the leader's entry: builds the initial stack and enters the executable */
void zkcl_linux_start(void *p1, void *p2, void *p3);

/* replace the calling process's executable; returns only on failure */
int zkcl_linux_exec_self(const char *path);

/* the default terminal attributes */
void zkcl_linux_termios_init(struct zkcl_linux_termios *t);

/* give a not yet started fork() child the calling process's state */
int zkcl_linux_process_clone(k_tid_t child);

#endif /* ZEPHYR_SUBSYS_KCL_LINUX_KCL_LINUX_H_ */
