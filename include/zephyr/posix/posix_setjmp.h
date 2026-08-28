/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief \<setjmp.h\>: POSIX signal-jump extensions
 *
 * Provides sigsetjmp() and siglongjmp(), the POSIX_SIGNAL_JUMP Option Group,
 * on top of the C library's setjmp() and longjmp(). The C library's
 * \<setjmp.h\> includes this header.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/setjmp.h.html">
 *      POSIX.1-2017 &lt;setjmp.h&gt;</a>
 */

#ifndef ZEPHYR_INCLUDE_POSIX_POSIX_SETJMP_H_
#define ZEPHYR_INCLUDE_POSIX_POSIX_SETJMP_H_

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#include <setjmp.h>
#include <signal.h>

#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __sigjmp_buf_defined
#define __sigjmp_buf_defined
/**
 * @brief Jump buffer for sigsetjmp() and siglongjmp()
 *
 * Holds the saved calling environment and, when requested, the saved signal
 * mask.
 *
 * @ingroup posix_option_group_signal_jump
 */
typedef struct __sigjmp_buf {
	jmp_buf __env;
	sigset_t __mask;
	int __savemask;
} sigjmp_buf[1];
#endif

/* saves the signal mask for sigsetjmp() and always returns 0 (internal) */
int __sigjmp_save(struct __sigjmp_buf *env, int savemask);

#if defined(__DOXYGEN__)
/**
 * @brief Save the calling environment and, optionally, the signal mask.
 *
 * When @a savemask is non-zero, the calling thread's signal mask is saved in
 * @a env and siglongjmp() restores it. sigsetjmp() is a macro, as permitted
 * by POSIX, so that the environment is saved in the caller's stack frame.
 *
 * @param env Buffer in which the calling environment is saved
 * @param savemask Save the signal mask in @a env when non-zero
 *
 * @retval 0 when returning directly from the sigsetjmp() invocation
 * @retval non-zero when returning via siglongjmp()
 *
 * @ingroup posix_option_group_signal_jump
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigsetjmp.html">sigsetjmp</a>
 */
int sigsetjmp(sigjmp_buf env, int savemask);
#else
#define sigsetjmp(env, savemask) (__sigjmp_save((env), (savemask)), setjmp((env)->__env))
#endif

/**
 * @brief Restore the environment saved by sigsetjmp().
 *
 * Control returns from the corresponding sigsetjmp() invocation with return
 * value @a val (or 1 when @a val is 0). When the environment was saved with a
 * non-zero savemask, the saved signal mask is restored first.
 *
 * @param env Buffer holding the environment saved by sigsetjmp()
 * @param val Value for the corresponding sigsetjmp() invocation to return
 *
 * @ingroup posix_option_group_signal_jump
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/functions/siglongjmp.html">siglongjmp</a>
 */
FUNC_NORETURN void siglongjmp(sigjmp_buf env, int val);

#ifdef __cplusplus
}
#endif

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#endif /* ZEPHYR_INCLUDE_POSIX_POSIX_SETJMP_H_ */
