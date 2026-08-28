/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Alias the POSIX in6addr_any / in6addr_loopback objects onto Zephyr's
 * net_in6addr_any / net_in6addr_loopback so the two spellings share storage
 * instead of each carrying a copy. PROVIDE yields to a stronger definition
 * (e.g. a host C library under native_sim).
 */
PROVIDE(in6addr_any = net_in6addr_any);
PROVIDE(in6addr_loopback = net_in6addr_loopback);
