/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_SHARED_TTY_FIXTURE_H_
#define POSIX_TESTS_SHARED_TTY_FIXTURE_H_

#include <stdbool.h>

/*
 * The terminal a suite tests against: the console under Zephyr, where it is the
 * one terminal; the slave side of a pseudo-terminal under the host C library,
 * where the standard descriptors are twister's pipes. With @p controlling the
 * host terminal becomes the controlling terminal of the test process, as the
 * console is under Zephyr.
 */
int posix_test_tty_open(bool controlling);
void posix_test_tty_close(void);

#endif /* POSIX_TESTS_SHARED_TTY_FIXTURE_H_ */
