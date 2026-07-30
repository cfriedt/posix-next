/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TESTS_POSIX_SHARED_NANOSLEEP_COMMON_H_
#define TESTS_POSIX_SHARED_NANOSLEEP_COMMON_H_

#include <stdint.h>
#include <time.h>

#define SELECT_NANOSLEEP       1
#define SELECT_CLOCK_NANOSLEEP 0

void common_lower_bound_check(int selection, clockid_t clock_id, int flags, const uint32_t s,
			      uint32_t ns);
void common_errors(int selection, clockid_t clock_id, int flags);
int select_nanosleep(int selection, clockid_t clock_id, int flags, const struct timespec *rqtp,
		     struct timespec *rmtp);

#endif /* TESTS_POSIX_SHARED_NANOSLEEP_COMMON_H_ */
