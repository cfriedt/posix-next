/*
 * Copyright (c) 2024 Google LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include <zephyr/drivers/entropy.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>

#define ENTROPY_NODE DEVICE_DT_GET_OR_NULL(DT_CHOSEN(zephyr_entropy))

int getentropy(void *buffer, size_t length)
{
	const struct device *const entropy = ENTROPY_NODE;

	__ASSERT_NO_MSG(buffer != NULL);

	if (length > 256) {
		errno = EIO;
		return -1;
	}

	if (entropy == NULL || !device_is_ready(entropy)) {
#if defined(CONFIG_CSPRNG_ENABLED) || defined(CONFIG_TEST_CSPRNG_GENERATOR)
		/* no entropy device: the kernel's (cryptographically secure) generator */
		if (sys_csrand_get(buffer, length) == 0) {
			return 0;
		}
#endif
		errno = EIO;
		return -1;
	}

	/*
	 * getentropy() uses size_t to represent buffer size, but Zephyr uses
	 * uint16_t. The length check above allows us to safely cast without
	 * overflow.
	 */
	if (entropy_get_entropy(entropy, buffer, (uint16_t)length)) {
		errno = EIO;
		return -1;
	}

	return 0;
}
