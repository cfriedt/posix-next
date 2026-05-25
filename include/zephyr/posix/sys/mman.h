/*
 * Copyright (c) 2024, Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX memory management (<sys/mman.h>)
 *
 * Provides memory mapping, shared memory objects, and memory locking.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_mman.h.html">
 *      POSIX.1-2017 &lt;sys/mman.h&gt;</a>
 *
 */

#ifndef ZEPHYR_INCLUDE_ZEPHYR_POSIX_SYS_MMAN_H_
#define ZEPHYR_INCLUDE_ZEPHYR_POSIX_SYS_MMAN_H_

#include <stddef.h>
#include <sys/types.h>

/** @brief Pages may not be accessed. @ingroup posix_option_group_mapped_files */
#define PROT_NONE  0x0
/** @brief Pages may be read. @ingroup posix_option_group_mapped_files */
#define PROT_READ  0x1
/** @brief Pages may be written. @ingroup posix_option_group_mapped_files */
#define PROT_WRITE 0x2
/** @brief Pages may be executed. @ingroup posix_option_group_mapped_files */
#define PROT_EXEC  0x4

/** @brief Changes are shared between all mappings of the same object. @ingroup posix_option_group_mapped_files */
#define MAP_SHARED  0x1
/** @brief Changes are private (copy-on-write). @ingroup posix_option_group_mapped_files */
#define MAP_PRIVATE 0x2
/** @brief Map at the exact address given in addr. @ingroup posix_option_group_mapped_files */
#define MAP_FIXED   0x4

/** @brief Anonymous mapping; fd argument is ignored. @ingroup posix_option_group_mapped_files */
#define MAP_ANONYMOUS 0x20

/** @brief Flush modified pages to the underlying file synchronously. @ingroup posix_option_group_mapped_files */
#define MS_SYNC       0x0
/** @brief Schedule writes; return immediately. @ingroup posix_option_group_mapped_files */
#define MS_ASYNC      0x1
/** @brief Invalidate cached data so subsequent reads reflect the file. @ingroup posix_option_group_mapped_files */
#define MS_INVALIDATE 0x2

/** @brief Value returned by mmap() on failure. @ingroup posix_option_group_mapped_files */
#define MAP_FAILED ((void *)-1)

/** @brief Lock all currently mapped pages into memory. @ingroup posix_option_memlock */
#define MCL_CURRENT 0
/** @brief Lock all future mappings into memory. @ingroup posix_option_memlock */
#define MCL_FUTURE  1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Lock a range of the calling process's address space into memory.
 * @ingroup posix_option_memlock_range
 * @param addr Base address of the region to lock.
 * @param len  Length of the region in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 */
int mlock(const void *addr, size_t len);

/**
 * @brief Lock all current and/or future memory mappings of the calling process.
 * @ingroup posix_option_memlock
 * @param flags MCL_CURRENT, MCL_FUTURE, or both.
 * @return 0 on success, or -1 with errno set on failure.
 */
int mlockall(int flags);

/**
 * @brief Map a file or device into memory.
 * @ingroup posix_option_group_mapped_files
 * @param addr   Suggested address (hint), or NULL.
 * @param len    Length of the mapping in bytes.
 * @param prot   Memory protection (PROT_* flags).
 * @param flags  Mapping type and options (MAP_* flags).
 * @param fildes File descriptor (-1 for anonymous mappings).
 * @param off    Offset within the file (must be page-aligned).
 * @return Base address of the mapping, or MAP_FAILED on failure.
 */
void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);

/**
 * @brief Synchronise a memory mapping with the underlying storage.
 * @ingroup posix_option_group_mapped_files
 * @param addr   Base address of the region (must be page-aligned).
 * @param length Length of the region in bytes.
 * @param flags  MS_SYNC, MS_ASYNC, or MS_INVALIDATE.
 * @return 0 on success, or -1 with errno set on failure.
 */
int msync(void *addr, size_t length, int flags);

/**
 * @brief Unlock a range of the calling process's address space.
 * @ingroup posix_option_memlock_range
 * @param addr Base address of the region to unlock.
 * @param len  Length of the region in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 */
int munlock(const void *addr, size_t len);

/**
 * @brief Unlock all memory locked by the calling process.
 * @ingroup posix_option_memlock
 * @return 0 on success, or -1 with errno set on failure.
 */
int munlockall(void);

/**
 * @brief Unmap a previously mapped region.
 * @ingroup posix_option_group_mapped_files
 * @param addr Base address of the mapping.
 * @param len  Length of the region in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 */
int munmap(void *addr, size_t len);

/**
 * @brief Open or create a shared memory object.
 * @ingroup posix_option_shared_memory_objects
 * @param name  Shared memory name (must start with '/').
 * @param oflag Open flags (O_RDONLY, O_RDWR, O_CREAT, O_EXCL, O_TRUNC).
 * @param mode  Permission bits applied if the object is created.
 * @return File descriptor for the shared memory object, or -1 on failure.
 */
int shm_open(const char *name, int oflag, mode_t mode);

/**
 * @brief Remove a shared memory object.
 * @ingroup posix_option_shared_memory_objects
 * @param name Shared memory name.
 * @return 0 on success, or -1 with errno set on failure.
 */
int shm_unlink(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_ZEPHYR_POSIX_SYS_MMAN_H_ */
