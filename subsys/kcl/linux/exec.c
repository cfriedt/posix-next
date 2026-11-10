/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>

#include <zephyr/fs/fs.h>
#include <zephyr/internal/syscall_handler.h>
#include <zephyr/kcl/linux/exec.h>
#include <zephyr/kernel.h>
#include <zephyr/llext/elf.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/process.h>
#include <zephyr/sys/util.h>

#include "kcl_linux.h"
#include <kernel_internal.h>

LOG_MODULE_REGISTER(kcl_linux, CONFIG_KCL_LINUX_LOG_LEVEL);

#define EI_CLASS 4
#define ELFCLASS32 1
#define ELFCLASS64 2
#define PT_INTERP 3
#define PF_X 1
#define PF_W 2
#define PF_R 4
#define EM_X86_64 62
#define EM_AARCH64 183
#define EM_RISCV 243
#define EM_386 3
#define EM_ARM 40

#if defined(CONFIG_X86_64)
#define ZKCL_LINUX_EM EM_X86_64
#elif defined(CONFIG_X86)
#define ZKCL_LINUX_EM EM_386
#elif defined(CONFIG_ARM64)
#define ZKCL_LINUX_EM EM_AARCH64
#elif defined(CONFIG_ARM)
#define ZKCL_LINUX_EM EM_ARM
#elif defined(CONFIG_RISCV)
#define ZKCL_LINUX_EM EM_RISCV
#endif

/* the program header as the ELF specification lays it out (llext's differs) */
#if defined(CONFIG_64BIT)
struct elf_phdr {
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
};
#else
struct elf_phdr {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
};
#endif

#define PHDRS_MAX 16
#define PARTS_MAX (PHDRS_MAX + 2)

#define PAGE ZKCL_LINUX_PAGE_SIZE

/*
 * Reaping runs under the process lock, where the heap must not be touched:
 * the state of a reaped process waits here until the next load.
 */
static struct zkcl_linux_process *graveyard[CONFIG_SYS_PROCESS_MAX + 4];
static struct k_spinlock graveyard_lock;

static void image_unref(struct zkcl_linux_image *img)
{
	if ((img != NULL) && (atomic_dec(&img->refs) == 1)) {
		k_free(img->mem);
		k_free(img);
	}
}

static void process_free(struct zkcl_linux_process *p)
{
	image_unref(p->image);
	if (p->stack_owned) {
		k_free(p->stack);
	}
	for (size_t i = 0; i < ARRAY_SIZE(p->maps); i++) {
		if ((p->maps[i].part.size != 0U) && p->maps[i].owned) {
			k_free((void *)p->maps[i].part.start);
		}
	}
	k_free(p);
}

static void graveyard_drain(void)
{
	while (true) {
		struct zkcl_linux_process *p = NULL;

		K_SPINLOCK(&graveyard_lock) {
			for (size_t i = 0; i < ARRAY_SIZE(graveyard); i++) {
				if (graveyard[i] != NULL) {
					p = graveyard[i];
					graveyard[i] = NULL;
					break;
				}
			}
		}
		if (p == NULL) {
			break;
		}
		process_free(p);
	}
}

void z_kcl_process_drop(struct k_process *proc)
{
	struct zkcl_linux_process *p = proc->compat;
	bool buried = false;

	proc->compat = NULL;
	if (p == NULL) {
		return;
	}
	K_SPINLOCK(&graveyard_lock) {
		for (size_t i = 0; i < ARRAY_SIZE(graveyard); i++) {
			if (graveyard[i] == NULL) {
				graveyard[i] = p;
				buried = true;
				break;
			}
		}
	}
	__ASSERT(buried, "process graveyard full");
}

static k_mem_partition_attr_t segment_attr(uint32_t flags)
{
	if ((flags & PF_X) != 0U) {
		return K_MEM_PARTITION_P_RX_U_RX;
	}
	if ((flags & PF_W) != 0U) {
		return K_MEM_PARTITION_P_RW_U_RW;
	}
	return K_MEM_PARTITION_P_RO_U_RO;
}

static int read_at(struct fs_file_t *f, size_t off, void *buf, size_t len)
{
	int ret = fs_seek(f, off, FS_SEEK_SET);

	while ((ret == 0) && (len > 0)) {
		ssize_t n = fs_read(f, buf, len);

		if (n <= 0) {
			return (n < 0) ? (int)n : -ENOEXEC;
		}
		buf = (uint8_t *)buf + n;
		len -= n;
	}

	return ret;
}

/* partition attributes are an integer or a structure, by architecture */
static bool attr_same(k_mem_partition_attr_t a, k_mem_partition_attr_t b)
{
	return memcmp(&a, &b, sizeof(a)) == 0;
}

/*
 * The loadable segments as page ranges: a segment's range ends where the next
 * one starts, the last runs to the end of the image plus the brk area, and
 * adjacent ranges of equal access are one partition.
 */
static size_t segment_partitions(const struct elf_phdr *ph, size_t n, uintptr_t base,
				 uintptr_t brk_end, struct k_mem_partition *parts)
{
	size_t count = 0;

	for (size_t i = 0; i < n; i++) {
		uintptr_t start = ROUND_DOWN(base + ph[i].p_vaddr, PAGE);
		uintptr_t end = (i + 1 < n) ? ROUND_DOWN(base + ph[i + 1].p_vaddr, PAGE)
					    : brk_end;
		k_mem_partition_attr_t attr = segment_attr(ph[i].p_flags);

		if (end <= start) {
			continue;
		}
		if ((count > 0) && attr_same(parts[count - 1].attr, attr) &&
		    (parts[count - 1].start + parts[count - 1].size == start)) {
			parts[count - 1].size += end - start;
			continue;
		}
		parts[count].start = start;
		parts[count].size = end - start;
		parts[count].attr = attr;
		count++;
	}

	return count;
}

/* load a static, position-independent executable into a fresh state */
static int image_load(const char *path, struct k_mem_domain *domain,
		      struct zkcl_linux_process **out)
{
	struct zkcl_linux_process *p = NULL;
	struct k_mem_partition parts[PARTS_MAX];
	struct elf_phdr *ph = NULL;
	struct fs_file_t f;
	elf_ehdr_t ehdr;
	uintptr_t lo = UINTPTR_MAX;
	uintptr_t hi = 0;
	size_t nload = 0;
	size_t nparts;
	size_t added = 0;
	int ret;

	graveyard_drain();

	fs_file_t_init(&f);
	ret = fs_open(&f, path, FS_O_READ);
	if (ret < 0) {
		return -ENOENT;
	}
	ret = read_at(&f, 0, &ehdr, sizeof(ehdr));
	if (ret < 0) {
		LOG_ERR("%s: reading the header: %d", path, ret);
		goto out;
	}
	ret = -ENOEXEC;
	if ((memcmp(ehdr.e_ident, "\177ELF", 4) != 0) ||
	    (ehdr.e_ident[EI_CLASS] != (IS_ENABLED(CONFIG_64BIT) ? ELFCLASS64 : ELFCLASS32)) ||
	    (ehdr.e_machine != ZKCL_LINUX_EM) || (ehdr.e_phentsize != sizeof(struct elf_phdr)) ||
	    (ehdr.e_phnum == 0U) || (ehdr.e_phnum > PHDRS_MAX)) {
		LOG_ERR("%s: not an executable for this machine (class %u, machine %u, %u headers of %u)",
			path, ehdr.e_ident[EI_CLASS], ehdr.e_machine, ehdr.e_phnum, ehdr.e_phentsize);
		goto out;
	}
	if (ehdr.e_type != ET_DYN) {
		/* a fixed-address executable needs an address space of its own */
		LOG_ERR("%s: not position independent", path);
		goto out;
	}

	ph = k_malloc(ehdr.e_phnum * sizeof(*ph));
	if (ph == NULL) {
		ret = -ENOMEM;
		goto out;
	}
	ret = read_at(&f, ehdr.e_phoff, ph, ehdr.e_phnum * sizeof(*ph));
	if (ret < 0) {
		goto out;
	}
	ret = -ENOEXEC;
	for (size_t i = 0; i < ehdr.e_phnum; i++) {
		if (ph[i].p_type == PT_INTERP) {
			LOG_ERR("%s: dynamically linked", path);
			goto out;
		}
		if (ph[i].p_type != PT_LOAD) {
			continue;
		}
		if (((nload > 0) && (ph[i].p_vaddr < ph[nload - 1].p_vaddr)) ||
		    (ph[i].p_filesz > ph[i].p_memsz)) {
			LOG_ERR("%s: malformed segment %zu", path, i);
			goto out;
		}
		ph[nload++] = ph[i];
		lo = MIN(lo, ROUND_DOWN(ph[i].p_vaddr, PAGE));
		hi = MAX(hi, ROUND_UP(ph[i].p_vaddr + ph[i].p_memsz, PAGE));
	}
	if (nload == 0) {
		LOG_ERR("%s: nothing to load", path);
		goto out;
	}

	p = k_calloc(1, sizeof(*p));
	if (p == NULL) {
		ret = -ENOMEM;
		goto out;
	}
	p->image = k_calloc(1, sizeof(*p->image));
	p->stack_size = ROUND_UP(CONFIG_KCL_LINUX_STACK_SIZE, PAGE);
	p->stack = k_aligned_alloc(PAGE, p->stack_size);
	p->stack_owned = true;
	if ((p->image == NULL) || (p->stack == NULL)) {
		ret = -ENOMEM;
		goto out;
	}
	p->image->refs = ATOMIC_INIT(1);
	p->image->size = (hi - lo) + CONFIG_KCL_LINUX_BRK_SIZE;
	p->image->mem = k_aligned_alloc(PAGE, p->image->size);
	if (p->image->mem == NULL) {
		ret = -ENOMEM;
		goto out;
	}
	memset(p->image->mem, 0, p->image->size);
	p->base = (uintptr_t)p->image->mem - lo;
	for (size_t i = 0; i < nload; i++) {
		ret = read_at(&f, ph[i].p_offset, (void *)(p->base + ph[i].p_vaddr),
			      ph[i].p_filesz);
		if (ret < 0) {
			goto out;
		}
	}
	p->entry = p->base + ehdr.e_entry;
	p->phdr = p->base + ehdr.e_phoff;
	p->phnum = ehdr.e_phnum;
	p->phentsize = ehdr.e_phentsize;
	p->brk_start = p->base + hi;
	p->brk = p->brk_start;
	p->brk_end = (uintptr_t)p->image->mem + p->image->size;
	sys_rand_get(p->random, sizeof(p->random));
	for (size_t i = 0; i < ARRAY_SIZE(p->socks); i++) {
		p->socks[i].fd = -1;
	}
	zkcl_linux_termios_init(&p->termios);

	nparts = segment_partitions(ph, nload, p->base, p->brk_end, parts);
	if ((nparts == 0) || !attr_same(parts[nparts - 1].attr, K_MEM_PARTITION_P_RW_U_RW)) {
		/* no writable tail to grow into: the brk area is its own partition */
		parts[nparts].start = p->brk_start;
		parts[nparts].size = p->brk_end - p->brk_start;
		parts[nparts].attr = K_MEM_PARTITION_P_RW_U_RW;
		nparts++;
	}
	parts[nparts].start = (uintptr_t)p->stack;
	parts[nparts].size = p->stack_size;
	parts[nparts].attr = K_MEM_PARTITION_P_RW_U_RW;
	nparts++;
	if (nparts > ARRAY_SIZE(p->parts)) {
		LOG_ERR("%s: %zu partitions", path, nparts);
		ret = -ENOEXEC;
		goto out;
	}
	for (; added < nparts; added++) {
		ret = k_mem_domain_add_partition(domain, &parts[added]);
		if (ret != 0) {
			LOG_ERR("%s: partition %zu (%zu bytes at %lx): %d", path, added,
				parts[added].size, (unsigned long)parts[added].start, ret);
			ret = -ENOMEM;
			goto out;
		}
	}
	memcpy(p->parts, parts, nparts * sizeof(parts[0]));
	p->nparts = nparts;

	LOG_DBG("%s: %zu segments at %p (bias %lx), entry %lx, stack %p", path, nload,
		p->image->mem, (unsigned long)p->base, (unsigned long)p->entry, p->stack);
	*out = p;
	ret = 0;
out:
	if ((ret != 0) && (p != NULL)) {
		for (size_t i = 0; i < added; i++) {
			(void)k_mem_domain_remove_partition(domain, &parts[i]);
		}
		process_free(p);
	}
	k_free(ph);
	(void)fs_close(&f);

	return ret;
}

int z_kcl_exec_load(struct k_process *proc, k_tid_t leader, const char *path,
		    struct sys_process_start *start)
{
	struct k_mem_domain *domain = leader->mem_domain_info.mem_domain;
	struct zkcl_linux_process *p;
	int ret;

	if (((leader->base.user_options & K_USER) == 0U) || (domain == NULL) ||
	    (domain == &k_mem_domain_default)) {
		/* a Linux executable runs in user mode, in a domain of its own */
		return -EINVAL;
	}
	ret = image_load(path, domain, &p);
	if (ret != 0) {
		return ret;
	}
	proc->compat = p;
	start->entry = zkcl_linux_start;

	return 0;
}

/*
 * execve(): the new executable's segments join the domain beside the old
 * one's, which then leave it; the old state waits in the graveyard like a
 * reaped process's, and the leader restarts on the new image. Descriptors,
 * their socket state included, stay, minus the close-on-exec ones.
 */
int zkcl_linux_exec_self(const char *path)
{
	struct k_process *proc = _current->process;
	struct k_mem_domain *domain = _current->mem_domain_info.mem_domain;
	struct zkcl_linux_process *old = zkcl_linux_current();
	struct zkcl_linux_process *p;
	int ret;

	if ((old == NULL) || (domain == NULL)) {
		return -EINVAL;
	}
	/* the restart runs the layer's own start code, which makes native system calls */
	arch_user_syscall_abi_set(_current, ARCH_USER_SYSCALL_ABI_NATIVE);
	ret = image_load(path, domain, &p);
	if (ret != 0) {
		return ret;
	}
	for (size_t i = 0; i < old->nparts; i++) {
		(void)k_mem_domain_remove_partition(domain, &old->parts[i]);
	}
	for (size_t i = 0; i < ARRAY_SIZE(old->maps); i++) {
		if (old->maps[i].part.size != 0U) {
			(void)k_mem_domain_remove_partition(domain, &old->maps[i].part);
		}
	}
	memcpy(p->socks, old->socks, sizeof(p->socks));
	p->termios = old->termios;
	z_kcl_process_drop(proc);
	proc->compat = p;

	return 0;
}

/*
 * A fork() child shares the image and runs on copies of everything writable
 * (the process layer copies the domain's frames at the same addresses), so
 * its state is the parent's with nothing of its own to free but later maps.
 */
int zkcl_linux_process_clone(k_tid_t child)
{
	struct zkcl_linux_process *src = zkcl_linux_current();
	struct zkcl_linux_process *p;

	if (src == NULL) {
		return -EINVAL;
	}
	graveyard_drain();
	p = k_malloc(sizeof(*p));
	if (p == NULL) {
		return -ENOMEM;
	}
	memcpy(p, src, sizeof(*p));
	atomic_inc(&p->image->refs);
	p->stack_owned = false;
	for (size_t i = 0; i < ARRAY_SIZE(p->maps); i++) {
		p->maps[i].owned = false;
		p->maps[i].fd = -1;
	}
	child->process->compat = p;
#ifdef CONFIG_USER_THREAD_POINTER
	arch_user_thread_pointer_set(child, arch_user_thread_pointer_get(_current));
#endif /* CONFIG_USER_THREAD_POINTER */
	/* the child resumes in Linux code, as its parent was running when it forked */
	arch_user_syscall_abi_set(child, ARCH_USER_SYSCALL_ABI_LINUX);

	return 0;
}

int z_impl_zkcl_linux_process_info(struct zkcl_linux_process_info *info)
{
	struct zkcl_linux_process *p = zkcl_linux_current();

	if (p == NULL) {
		return -ENOENT;
	}
	info->entry = p->entry;
	info->phdr = p->phdr;
	info->phnum = p->phnum;
	info->phentsize = p->phentsize;
	info->stack = (uintptr_t)p->stack;
	info->stack_size = p->stack_size;
	memcpy(info->random, p->random, sizeof(info->random));

	return 0;
}

static inline int z_vrfy_zkcl_linux_process_info(struct zkcl_linux_process_info *info)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(info, sizeof(*info)));
	return z_impl_zkcl_linux_process_info(info);
}
#include <zephyr/syscalls/zkcl_linux_process_info_mrsh.c>

void z_impl_zkcl_linux_enter(void)
{
	arch_user_syscall_abi_set(_current, ARCH_USER_SYSCALL_ABI_LINUX);
}

static inline void z_vrfy_zkcl_linux_enter(void)
{
	z_impl_zkcl_linux_enter();
}
#include <zephyr/syscalls/zkcl_linux_enter_mrsh.c>
