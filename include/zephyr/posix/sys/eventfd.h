/*
 * Copyright (c) 2020 Tobias Svehagen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Linux-compatible event notification file descriptor (<sys/eventfd.h>)
 *
 * eventfd provides a lightweight, kernel-managed counter that can be used for
 * event notification between threads or between a kernel component and
 * user space.  It integrates with poll/select/epoll.
 *
 * @note eventfd is a Linux extension, not part of POSIX.1-2017, but is
 *       widely available on Linux and implemented here for compatibility.
 *
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_EVENTFD_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_EVENTFD_H_

#include <zephyr/zvfs/eventfd.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Semaphore-mode flag: each read decrements the counter by 1 instead of resetting to 0.  @ingroup posix_option_group_device_io*/
#define EFD_SEMAPHORE ZVFS_EFD_SEMAPHORE
/** @brief Non-blocking flag: reads and writes return EAGAIN instead of blocking.  @ingroup posix_option_group_device_io*/
#define EFD_NONBLOCK  ZVFS_EFD_NONBLOCK

/** @brief Counter value type for eventfd operations.  @ingroup posix_option_group_device_io*/
typedef zvfs_eventfd_t eventfd_t;

/**
 * @brief Create a file descriptor for event notification.
 * @ingroup posix_option_group_device_io
 *
 * The returned file descriptor can be used with POSIX read()/write() or with
 * eventfd_read()/eventfd_write().  It also integrates with poll(), allowing a
 * writing thread to wake a polling thread simply by writing to the eventfd.
 *
 * When using read()/write(), the buffer size must be exactly 8 bytes or the
 * call will fail with @c EINVAL.
 *
 * @param initval Initial counter value.
 * @param flags   0, EFD_SEMAPHORE, EFD_NONBLOCK, or their combination.
 * @return New eventfd file descriptor on success, -1 with errno set on failure.
 */
int eventfd(unsigned int initval, int flags);

/**
 * @brief Read the current counter value from an eventfd.
 * @ingroup posix_option_group_device_io
 *
 * In normal mode the counter is reset to 0; in EFD_SEMAPHORE mode it is
 * decremented by 1.  Blocks if the counter is 0 (unless EFD_NONBLOCK).
 *
 * @param fd    Eventfd file descriptor.
 * @param value Output: counter value read.
 * @return 0 on success, -1 with errno set on failure.
 */
int eventfd_read(int fd, eventfd_t *value);

/**
 * @brief Add a value to an eventfd counter.
 * @ingroup posix_option_group_device_io
 *
 * Wakes any thread blocked in eventfd_read(), read(), or poll() on @p fd.
 *
 * @param fd    Eventfd file descriptor.
 * @param value Value to add to the counter.
 * @return 0 on success, -1 with errno set on failure.
 */
int eventfd_write(int fd, eventfd_t value);


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_EVENTFD_H_ */
