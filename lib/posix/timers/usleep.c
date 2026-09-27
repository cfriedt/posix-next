/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2018 Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <zephyr/kernel.h>

#if !defined(_USECONDS_T_DECLARED) && !defined(__useconds_t_defined)
typedef unsigned long useconds_t;
#define _USECONDS_T_DECLARED
#define __useconds_t_defined
#endif

int usleep(useconds_t useconds)
{
	int32_t rem;

	if (useconds >= USEC_PER_SEC) {
		errno = EINVAL;
		return -1;
	}

	rem = k_usleep(useconds);
	__ASSERT_NO_MSG(rem >= 0);
	if (rem > 0) {
		/* sleep was interrupted by a call to k_wakeup() */
		errno = EINTR;
		return -1;
	}

	return 0;
}
