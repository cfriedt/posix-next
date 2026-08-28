/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <signal.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/signal.h>
#include <zephyr/sys/math_extras.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>

#define SIGSET_NLONGS (sizeof(sigset_t) / sizeof(unsigned long))
#define K_SIGSET_NLONGS (sizeof(struct k_sig_set) / sizeof(unsigned long))

/*
 * Upper bound for standard (non-RT) signal numbers in the slow-path lookup
 * tables. ISO C/POSIX.1 assigns 1..31; tables are (Z_SIG_STD_MAP_MAX + 1)
 * bytes regardless of SIGNAL_SET_SIZE or libc signal.h values.
 * 32 and 33 are reserved for cancel and reserved signals.
 */
#define Z_SIG_STD_MAP_MAX 33U

BUILD_ASSERT(K_SIG_RTMIN > (int)Z_SIG_STD_MAP_MAX);

/*
 * Minimal libc's signal numbering is Linux-aligned, exactly matching the kernel's; the identity
 * fast path in z_sig_set_from_posix()/z_sig_set_to_posix() depends on it.
 */
#ifdef CONFIG_MINIMAL_LIBC
BUILD_ASSERT(SIGHUP == K_SIG_HUP);
BUILD_ASSERT(SIGINT == K_SIG_INT);
BUILD_ASSERT(SIGQUIT == K_SIG_QUIT);
BUILD_ASSERT(SIGILL == K_SIG_ILL);
BUILD_ASSERT(SIGTRAP == K_SIG_TRAP);
BUILD_ASSERT(SIGABRT == K_SIG_ABRT);
BUILD_ASSERT(SIGBUS == K_SIG_BUS);
BUILD_ASSERT(SIGFPE == K_SIG_FPE);
BUILD_ASSERT(SIGKILL == K_SIG_KILL);
BUILD_ASSERT(SIGUSR1 == K_SIG_USR1);
BUILD_ASSERT(SIGSEGV == K_SIG_SEGV);
BUILD_ASSERT(SIGUSR2 == K_SIG_USR2);
BUILD_ASSERT(SIGPIPE == K_SIG_PIPE);
BUILD_ASSERT(SIGALRM == K_SIG_ALRM);
BUILD_ASSERT(SIGTERM == K_SIG_TERM);
BUILD_ASSERT(SIGCHLD == K_SIG_CHLD);
BUILD_ASSERT(SIGCONT == K_SIG_CONT);
BUILD_ASSERT(SIGSTOP == K_SIG_STOP);
BUILD_ASSERT(SIGTSTP == K_SIG_TSTP);
BUILD_ASSERT(SIGTTIN == K_SIG_TTIN);
BUILD_ASSERT(SIGTTOU == K_SIG_TTOU);
BUILD_ASSERT(SIGURG == K_SIG_URG);
BUILD_ASSERT(SIGXCPU == K_SIG_XCPU);
BUILD_ASSERT(SIGXFSZ == K_SIG_XFSZ);
BUILD_ASSERT(SIGVTALRM == K_SIG_VTALRM);
BUILD_ASSERT(SIGPROF == K_SIG_PROF);
BUILD_ASSERT(SIGPOLL == K_SIG_POLL);
BUILD_ASSERT(SIGSYS == K_SIG_SYS);
BUILD_ASSERT(SIGRTMIN == K_SIG_RTMIN);
#endif /* CONFIG_MINIMAL_LIBC */

/*
 * Standard-signal maps indexed by ISO C/POSIX.1 signal number (1..31), not libc
 * SIG* macro values — sigset_t bit (n - 1) always refers to signo n. Gaps at
 * 16/28/30. Real-time signals use a linear offset in z_sig_map().
 */
static const uint8_t z_posix_to_k_std[Z_SIG_STD_MAP_MAX + 1] = {
	[0] = 0,
	[SIGHUP] = K_SIG_HUP,
	[SIGINT] = K_SIG_INT,
	[SIGQUIT] = K_SIG_QUIT,
	[SIGILL] = K_SIG_ILL,
	[SIGTRAP] = K_SIG_TRAP,
	[SIGABRT] = K_SIG_ABRT,
	[SIGBUS] = K_SIG_BUS,
	[SIGFPE] = K_SIG_FPE,
	[SIGKILL] = K_SIG_KILL,
	[SIGUSR1] = K_SIG_USR1,
	[SIGSEGV] = K_SIG_SEGV,
	[SIGUSR2] = K_SIG_USR2,
	[SIGPIPE] = K_SIG_PIPE,
	[SIGALRM] = K_SIG_ALRM,
	[SIGTERM] = K_SIG_TERM,
	/* 16: SIGSTKFLT — no K_SIG_* */
	[SIGCHLD] = K_SIG_CHLD,
	[SIGCONT] = K_SIG_CONT,
	[SIGSTOP] = K_SIG_STOP,
	[SIGTSTP] = K_SIG_TSTP,
	[SIGTTIN] = K_SIG_TTIN,
	[SIGTTOU] = K_SIG_TTOU,
	[SIGURG] = K_SIG_URG,
	[SIGXCPU] = K_SIG_XCPU,
	[SIGXFSZ] = K_SIG_XFSZ,
	[SIGVTALRM] = K_SIG_VTALRM,
	[SIGPROF] = K_SIG_PROF,
	/* 28: SIGWINCH — no K_SIG_* */
	[SIGPOLL] = K_SIG_POLL,
	/* 30: SIGPWR — no K_SIG_* */
	[SIGSYS] = K_SIG_SYS,
	[32] = K_SIG_CANCEL,
	[33] = K_SIG_RESERVED,
};

static const uint8_t z_k_to_posix_std[Z_SIG_STD_MAP_MAX + 1] = {
	[0] = 0,
	[K_SIG_HUP] = SIGHUP,
	[K_SIG_INT] = SIGINT,
	[K_SIG_QUIT] = SIGQUIT,
	[K_SIG_ILL] = SIGILL,
	[K_SIG_TRAP] = SIGTRAP,
	[K_SIG_ABRT] = SIGABRT,
	[K_SIG_BUS] = SIGBUS,
	[K_SIG_FPE] = SIGFPE,
	[K_SIG_KILL] = SIGKILL,
	[K_SIG_USR1] = SIGUSR1,
	[K_SIG_SEGV] = SIGSEGV,
	[K_SIG_USR2] = SIGUSR2,
	[K_SIG_PIPE] = SIGPIPE,
	[K_SIG_ALRM] = SIGALRM,
	[K_SIG_TERM] = SIGTERM,
	/* 16: SIGSTKFLT — no K_SIG_* */
	[K_SIG_CHLD] = SIGCHLD,
	[K_SIG_CONT] = SIGCONT,
	[K_SIG_STOP] = SIGSTOP,
	[K_SIG_TSTP] = SIGTSTP,
	[K_SIG_TTIN] = SIGTTIN,
	[K_SIG_TTOU] = SIGTTOU,
	[K_SIG_URG] = SIGURG,
	[K_SIG_XCPU] = SIGXCPU,
	[K_SIG_XFSZ] = SIGXFSZ,
	[K_SIG_VTALRM] = SIGVTALRM,
	[K_SIG_PROF] = SIGPROF,
	/* 28: SIGWINCH — no K_SIG_* */
	[K_SIG_POLL] = SIGPOLL,
	/* 30: SIGPWR — no K_SIG_* */
	[K_SIG_SYS] = SIGSYS,
	[K_SIG_CANCEL] = 32,
	[K_SIG_RESERVED] = 33,
};

static int z_sig_map(int signo, const uint8_t *std_map, int rt_lo, int rt_hi, int rt_dst_base)
{
	if ((signo >= rt_lo) && (signo <= rt_hi)) {
		return signo - rt_lo + rt_dst_base;
	}

	if ((signo < 0) || (signo > (int)Z_SIG_STD_MAP_MAX)) {
		return -1;
	}

	return std_map[signo];
}

static void sig_words_remap(const unsigned long *src, size_t src_nlongs, unsigned long *dst,
			    size_t dst_nlongs, int (*map)(int))
{
	memset(dst, 0, dst_nlongs * sizeof(unsigned long));

	for (size_t wi = 0; wi < src_nlongs; wi++) {
		unsigned long word = src[wi];

		while (word != 0UL) {
			const int bit = IS_ENABLED(CONFIG_64BIT)
						? u64_count_trailing_zeros(word)
						: u32_count_trailing_zeros((uint32_t)word);
			const int from = (int)(wi * BITS_PER_LONG + bit + 1);
			const int to = map(from);

			/*
			 * A signal with no counterpart is dropped rather than carried across by
			 * its number. The two namespaces have different gaps -- the kernel has no
			 * name for 16, 28, or 30, and a C library is free to number signals however
			 * it likes -- so an unmapped number aliases onto an unrelated signal. Under
			 * the Cygwin numbering picolibc and newlib use, carrying kernel signal 30
			 * across would land on SIGUSR1.
			 */
			if (to > 0) {
				const int db = (to - 1) & BIT_MASK(LOG2(BITS_PER_LONG));
				const size_t di = (to - 1) / BITS_PER_LONG;

				if (di < dst_nlongs) {
					if (IS_ENABLED(CONFIG_64BIT)) {
						dst[di] |= BIT64(db);
					} else {
						dst[di] |= BIT(db);
					}
				}
			}

			word &= word - 1UL;
		}
	}
}

__weak int z_sig_from_posix(int sig)
{
	return z_sig_map(sig, z_posix_to_k_std, SIGRTMIN, SIGRTMAX, K_SIG_RTMIN);
}

__weak int z_sig_to_posix(int sig)
{
	return z_sig_map(sig, z_k_to_posix_std, K_SIG_RTMIN, K_SIG_RTMIN + K_SIG_NUM_RT,
			 SIGRTMIN);
}

__weak struct k_sig_set *z_sig_set_from_posix_slow(const sigset_t *set, struct k_sig_set *buf)
{
	sig_words_remap((const unsigned long *)set, SIGSET_NLONGS, buf->sig, K_SIGSET_NLONGS,
			z_sig_from_posix);

	return buf;
}

__weak sigset_t *z_sig_set_to_posix_slow(const struct k_sig_set *kset, sigset_t *buf)
{
	sig_words_remap(kset->sig, K_SIGSET_NLONGS, (unsigned long *)buf, SIGSET_NLONGS,
			z_sig_to_posix);

	return buf;
}
