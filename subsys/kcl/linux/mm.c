/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Memory: the program break grows into the area reserved after the
 * executable's data, and mappings are allocations added to the process's
 * domain, one partition each. A file mapping is a copy of the file's bytes;
 * a shared writable one keeps a duplicate of the descriptor and is written
 * back through it at munmap() and msync(), so the program may close its own.
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs.h>

#include "kcl_linux.h"

LOG_MODULE_DECLARE(kcl_linux, CONFIG_KCL_LINUX_LOG_LEVEL);

#define PAGE ZKCL_LINUX_PAGE_SIZE

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define MAP_PRIVATE 0x02
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20

#define LINUX_ENOMEM_PTR ((long)-ENOMEM)

ZKCL_LINUX_IMPL(brk)(unsigned long brk)
{
	struct zkcl_linux_process *p = zkcl_linux_current();

	if (p == NULL) {
		return -ENOMEM;
	}
	if ((brk >= p->brk_start) && (brk <= p->brk_end)) {
		p->brk = brk;
	}
	LOG_DBG("brk(%lx) -> %lx [%lx, %lx)", brk, (unsigned long)p->brk,
		(unsigned long)p->brk_start, (unsigned long)p->brk_end);

	return p->brk;
}

static k_mem_partition_attr_t prot_attr(unsigned long prot)
{
	if ((prot & PROT_EXEC) != 0UL) {
		return K_MEM_PARTITION_P_RX_U_RX;
	}
	if ((prot & PROT_WRITE) != 0UL) {
		return K_MEM_PARTITION_P_RW_U_RW;
	}
	return K_MEM_PARTITION_P_RO_U_RO;
}

/* the file's bytes from off into mem: how many, or -errno */
static ssize_t map_file(int fd, unsigned long off, void *mem, size_t len)
{
	size_t done = 0;

	while (done < len) {
		off_t at = (off_t)(off + done);
		ssize_t n = zvfs_read_offset(fd, (uint8_t *)mem + done, len - done, &at);

		if (n < 0) {
			return (errno == EBADF) ? -EBADF : -ENODEV;
		}
		if (n == 0) {
			break;
		}
		done += (size_t)n;
	}

	return (ssize_t)done;
}

static void map_write_back(const struct zkcl_linux_mapping *map)
{
	const uint8_t *mem = (const uint8_t *)map->part.start;

	if ((map->fd < 0) || (zvfs_fd_entry_get(map->fd) != map->file)) {
		/* the duplicate is gone: the program closed it by number */
		return;
	}
	for (size_t done = 0; done < map->file_len;) {
		off_t at = (off_t)(map->off + done);
		ssize_t n = zvfs_write_offset(map->fd, mem + done, map->file_len - done, &at);

		if (n <= 0) {
			LOG_WRN("mapping of fd %d not written back: %d", map->fd, (n < 0) ? errno : 0);
			return;
		}
		done += (size_t)n;
	}
}

ZKCL_LINUX_IMPL(mmap)(unsigned long addr, unsigned long len, unsigned long prot,
				    unsigned long flags, unsigned long fd, unsigned long off)
{
	struct zkcl_linux_process *p = zkcl_linux_current();
	struct zkcl_linux_mapping *map = NULL;
	const bool file = (flags & MAP_ANONYMOUS) == 0UL;
	const bool shared_write = file && ((prot & PROT_WRITE) != 0UL) &&
				  ((flags & MAP_PRIVATE) == 0UL);
	ssize_t got = 0;
	void *mem;
	int ret;

	ARG_UNUSED(addr);
	if ((p == NULL) || (len == 0UL) || (len > (SIZE_MAX - PAGE)) ||
	    (file && ((off & (PAGE - 1)) != 0UL))) {
		return -EINVAL;
	}
	if ((flags & MAP_FIXED) != 0UL) {
		return -ENODEV;
	}
	if (file) {
		if ((int)fd < 0) {
			return -EBADF;
		}
	} else if (((flags & MAP_PRIVATE) == 0UL) || ((int)fd != -1)) {
		return -ENODEV;
	}
	for (size_t i = 0; i < ARRAY_SIZE(p->maps); i++) {
		if (p->maps[i].part.size == 0U) {
			map = &p->maps[i];
			break;
		}
	}
	if (map == NULL) {
		return -ENOMEM;
	}
	len = ROUND_UP(len, PAGE);
	mem = k_aligned_alloc(PAGE, len);
	if (mem == NULL) {
		return -ENOMEM;
	}
	memset(mem, 0, len);
	if (file) {
		got = map_file((int)fd, off, mem, len);
		if (got < 0) {
			k_free(mem);
			return (int)got;
		}
	}
	map->part.start = (uintptr_t)mem;
	map->part.size = len;
	map->part.attr = prot_attr(prot);
	map->owned = true;
	map->fd = -1;
	map->file = NULL;
	map->off = off;
	map->file_len = (size_t)got;
	if (shared_write) {
		map->fd = zvfs_dup((int)fd, 0);
		if (map->fd < 0) {
			k_free(mem);
			return -EMFILE;
		}
		(void)zvfs_fcntl(map->fd, F_SETFD, FD_CLOEXEC);
		map->file = zvfs_fd_entry_get(map->fd);
	}
	ret = k_mem_domain_add_partition(_current->mem_domain_info.mem_domain, &map->part);
	if (ret != 0) {
		map->part.size = 0;
		k_free(mem);
		return -ENOMEM;
	}
	LOG_DBG("mmap(%lu bytes, prot %lx, fd %d) -> %p", len, prot, (int)fd, mem);

	return (long)mem;
}

ZKCL_LINUX_IMPL(munmap)(unsigned long addr, size_t len)
{
	struct zkcl_linux_process *p = zkcl_linux_current();

	if (p == NULL) {
		return -EINVAL;
	}
	for (size_t i = 0; i < ARRAY_SIZE(p->maps); i++) {
		struct zkcl_linux_mapping *map = &p->maps[i];

		if ((map->part.size != 0U) && (map->part.start == addr) &&
		    (ROUND_UP(len, PAGE) == map->part.size)) {
			(void)k_mem_domain_remove_partition(_current->mem_domain_info.mem_domain,
							    &map->part);
			if (map->owned) {
				map_write_back(map);
				k_free((void *)map->part.start);
				if (map->fd >= 0) {
					(void)zvfs_close(map->fd);
				}
			}
			map->part.size = 0;
			map->owned = false;
			map->fd = -1;
			return 0;
		}
	}
	/* partial or unknown ranges cannot be unmapped */

	return -EINVAL;
}

ZKCL_LINUX_IMPL(msync)(unsigned long start, size_t len, int flags)
{
	struct zkcl_linux_process *p = zkcl_linux_current();

	ARG_UNUSED(flags);
	if ((p == NULL) || ((start & (PAGE - 1)) != 0UL)) {
		return -EINVAL;
	}
	for (size_t i = 0; i < ARRAY_SIZE(p->maps); i++) {
		const struct zkcl_linux_mapping *map = &p->maps[i];

		if ((map->part.size != 0U) && (start >= map->part.start) &&
		    ((start + len) <= (map->part.start + map->part.size))) {
			if (map->owned) {
				map_write_back(map);
			}
			return 0;
		}
	}

	return -ENOMEM;
}

ZKCL_LINUX_IMPL(mprotect)(unsigned long start, size_t len, unsigned long prot)
{
	struct zkcl_linux_process *p = zkcl_linux_current();

	ARG_UNUSED(prot);
	if (p == NULL) {
		return -EINVAL;
	}
	if ((start & (PAGE - 1)) != 0UL) {
		return -EINVAL;
	}
	/* access stays as loaded or mapped */
	ARG_UNUSED(len);

	return 0;
}

ZKCL_LINUX_IMPL(madvise)(unsigned long start, size_t len, int behavior)
{
	ARG_UNUSED(start);
	ARG_UNUSED(len);
	ARG_UNUSED(behavior);

	return 0;
}
