/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/llext/symbol.h>

/* a loadable object that is not an executable image: it exports no main() */
int not_main(void)
{
	return 0;
}
LL_EXTENSION_SYMBOL(not_main);
