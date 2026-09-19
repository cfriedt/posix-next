/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
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

/**
 * @brief Schedule writes; return immediately.
 * @ingroup posix_option_group_mapped_files
 */
#define MS_ASYNC      0x1
/**
 * @brief Invalidate cached data so subsequent reads reflect the file.
 * @ingroup posix_option_group_mapped_files
 */
#define MS_INVALIDATE 0x2
/**
 * @brief Flush modified pages to the underlying file synchronously.
 * @ingroup posix_option_group_mapped_files
 */
#define MS_SYNC       0x4

/** @brief Value returned by mmap() on failure. @ingroup posix_option_group_mapped_files */
#define MAP_FAILED ((void *)-1)

/** @brief Lock all currently mapped pages into memory. @ingroup posix_option_memlock */
#define MCL_CURRENT 0
/** @brief Lock all future mappings into memory. @ingroup posix_option_memlock */
#define MCL_FUTURE  1

#if defined(_POSIX_ADVISORY_INFO) || defined(__DOXYGEN__)
/** @brief No advice (default access pattern). @ingroup posix_option_advisory_info */
#define POSIX_MADV_NORMAL     0
/** @brief Memory will be accessed in random order. @ingroup posix_option_advisory_info */
#define POSIX_MADV_RANDOM     1
/** @brief Memory will be accessed sequentially. @ingroup posix_option_advisory_info */
#define POSIX_MADV_SEQUENTIAL 2
/** @brief Memory will be needed in the near future. @ingroup posix_option_advisory_info */
#define POSIX_MADV_WILLNEED   3
/** @brief Memory will not be accessed in the near future. @ingroup posix_option_advisory_info */
#define POSIX_MADV_DONTNEED   4
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Lock a range of the calling process's address space into memory.
 * @ingroup posix_option_memlock_range
 * @param addr Base address of the region to lock.
 * @param len  Length of the region in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/mlock.html
 */
int mlock(const void *addr, size_t len);

/**
 * @brief Lock all current and/or future memory mappings of the calling process.
 * @ingroup posix_option_memlock
 * @param flags MCL_CURRENT, MCL_FUTURE, or both.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/mlockall.html
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
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/mmap.html
 */
void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);

/**
 * @brief Change the protection of a memory mapping.
 * @ingroup posix_option_group_memory_protection
 * @param addr Base address of the region (must be page-aligned).
 * @param len  Length of the region in bytes.
 * @param prot New memory protection (PROT_* flags).
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/mprotect.html
 */
int mprotect(void *addr, size_t len, int prot);

/**
 * @brief Synchronise a memory mapping with the underlying storage.
 * @ingroup posix_option_synchronized_io
 * @param addr   Base address of the region (must be page-aligned).
 * @param length Length of the region in bytes.
 * @param flags  MS_SYNC, MS_ASYNC, or MS_INVALIDATE.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/msync.html
 */
int msync(void *addr, size_t length, int flags);

/**
 * @brief Unlock a range of the calling process's address space.
 * @ingroup posix_option_memlock_range
 * @param addr Base address of the region to unlock.
 * @param len  Length of the region in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/munlock.html
 */
int munlock(const void *addr, size_t len);

/**
 * @brief Unlock all memory locked by the calling process.
 * @ingroup posix_option_memlock
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/munlockall.html
 */
int munlockall(void);

/**
 * @brief Unmap a previously mapped region.
 * @ingroup posix_option_group_mapped_files
 * @param addr Base address of the mapping.
 * @param len  Length of the region in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/munmap.html
 */
int munmap(void *addr, size_t len);

#if defined(_POSIX_ADVISORY_INFO) || defined(__DOXYGEN__)
/**
 * @brief Declare an expected access pattern for a memory region.
 * @ingroup posix_option_advisory_info
 * @param addr   Start of the region.
 * @param len    Length of the region in bytes.
 * @param advice Access pattern hint (POSIX_MADV_*).
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_madvise.html
 */
int posix_madvise(void *addr, size_t len, int advice);
#endif

/**
 * @brief Open or create a shared memory object.
 * @ingroup posix_option_shared_memory_objects
 * @param name  Shared memory name (must start with '/').
 * @param oflag Open flags (O_RDONLY, O_RDWR, O_CREAT, O_EXCL, O_TRUNC).
 * @param mode  Permission bits applied if the object is created.
 * @return File descriptor for the shared memory object, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/shm_open.html
 */
int shm_open(const char *name, int oflag, mode_t mode);

/**
 * @brief Remove a shared memory object.
 * @ingroup posix_option_shared_memory_objects
 * @param name Shared memory name.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/shm_unlink.html
 */
int shm_unlink(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_ZEPHYR_POSIX_SYS_MMAN_H_ */
