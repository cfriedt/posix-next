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
#define K_SIGSET_NLONGS ARRAY_SIZE(((struct k_sig_set *)0)->sig)

/*
 * Upper bound for standard (non-RT) signal numbers in the slow-path lookup
 * tables. ISO C/POSIX.1 assigns 1..31; tables are (Z_SIG_STD_MAP_MAX + 1)
 * bytes regardless of SIGNAL_SET_SIZE or libc signal.h values.
 */
#define Z_SIG_STD_MAP_MAX 31U

BUILD_ASSERT((K_SIG_HUP <= Z_SIG_STD_MAP_MAX) && (K_SIG_INT <= Z_SIG_STD_MAP_MAX) &&
	     (K_SIG_QUIT <= Z_SIG_STD_MAP_MAX) && (K_SIG_ILL <= Z_SIG_STD_MAP_MAX) &&
	     (K_SIG_TRAP <= Z_SIG_STD_MAP_MAX) && (K_SIG_ABRT <= Z_SIG_STD_MAP_MAX) &&
	     (K_SIG_BUS <= Z_SIG_STD_MAP_MAX) && (K_SIG_FPE <= Z_SIG_STD_MAP_MAX) &&
	     (K_SIG_KILL <= Z_SIG_STD_MAP_MAX) && (K_SIG_USR1 <= Z_SIG_STD_MAP_MAX) &&
	     (K_SIG_SEGV <= Z_SIG_STD_MAP_MAX) && (K_SIG_USR2 <= Z_SIG_STD_MAP_MAX) &&
	     (K_SIG_PIPE <= Z_SIG_STD_MAP_MAX) && (K_SIG_ALRM <= Z_SIG_STD_MAP_MAX) &&
	     (K_SIG_TERM <= Z_SIG_STD_MAP_MAX) && (K_SIG_CHLD <= Z_SIG_STD_MAP_MAX) &&
	     (K_SIG_CONT <= Z_SIG_STD_MAP_MAX) && (K_SIG_STOP <= Z_SIG_STD_MAP_MAX) &&
	     (K_SIG_TSTP <= Z_SIG_STD_MAP_MAX) && (K_SIG_TTIN <= Z_SIG_STD_MAP_MAX) &&
	     (K_SIG_TTOU <= Z_SIG_STD_MAP_MAX) && (K_SIG_URG <= Z_SIG_STD_MAP_MAX) &&
	     (K_SIG_XCPU <= Z_SIG_STD_MAP_MAX) && (K_SIG_XFSZ <= Z_SIG_STD_MAP_MAX) &&
	     (K_SIG_VTALRM <= Z_SIG_STD_MAP_MAX) && (K_SIG_PROF <= Z_SIG_STD_MAP_MAX) &&
	     (K_SIG_POLL <= Z_SIG_STD_MAP_MAX) && (K_SIG_SYS <= Z_SIG_STD_MAP_MAX));

BUILD_ASSERT((SIGHUP <= Z_SIG_STD_MAP_MAX) && (SIGINT <= Z_SIG_STD_MAP_MAX) &&
	     (SIGQUIT <= Z_SIG_STD_MAP_MAX) && (SIGILL <= Z_SIG_STD_MAP_MAX) &&
	     (SIGTRAP <= Z_SIG_STD_MAP_MAX) && (SIGABRT <= Z_SIG_STD_MAP_MAX) &&
	     (SIGBUS <= Z_SIG_STD_MAP_MAX) && (SIGFPE <= Z_SIG_STD_MAP_MAX) &&
	     (SIGKILL <= Z_SIG_STD_MAP_MAX) && (SIGUSR1 <= Z_SIG_STD_MAP_MAX) &&
	     (SIGSEGV <= Z_SIG_STD_MAP_MAX) && (SIGUSR2 <= Z_SIG_STD_MAP_MAX) &&
	     (SIGPIPE <= Z_SIG_STD_MAP_MAX) && (SIGALRM <= Z_SIG_STD_MAP_MAX) &&
	     (SIGTERM <= Z_SIG_STD_MAP_MAX) && (SIGCHLD <= Z_SIG_STD_MAP_MAX) &&
	     (SIGCONT <= Z_SIG_STD_MAP_MAX) && (SIGSTOP <= Z_SIG_STD_MAP_MAX) &&
	     (SIGTSTP <= Z_SIG_STD_MAP_MAX) && (SIGTTIN <= Z_SIG_STD_MAP_MAX) &&
	     (SIGTTOU <= Z_SIG_STD_MAP_MAX) && (SIGURG <= Z_SIG_STD_MAP_MAX) &&
	     (SIGXCPU <= Z_SIG_STD_MAP_MAX) && (SIGXFSZ <= Z_SIG_STD_MAP_MAX) &&
	     (SIGVTALRM <= Z_SIG_STD_MAP_MAX) && (SIGPROF <= Z_SIG_STD_MAP_MAX) &&
	     (SIGPOLL <= Z_SIG_STD_MAP_MAX) && (SIGSYS <= Z_SIG_STD_MAP_MAX));

BUILD_ASSERT(SIGRTMIN > Z_SIG_STD_MAP_MAX);

/*
 * Standard-signal maps (gaps at SIGSTKFLT/SIGWINCH/SIGPWR). Real-time signals
 * use a linear offset in z_sig_map().
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
	/* SIGSTKFLT — no K_SIG_* */
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
	/* SIGWINCH — no K_SIG_* */
	[SIGPOLL] = K_SIG_POLL,
	/* SIGPWR — no K_SIG_* */
	[SIGSYS] = K_SIG_SYS,
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
	/* SIGSTKFLT — no K_SIG_* */
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
	/* SIGWINCH — no K_SIG_* */
	[K_SIG_POLL] = SIGPOLL,
	/* SIGPWR — no K_SIG_* */
	[K_SIG_SYS] = SIGSYS,
};

static int z_sig_map(int signo, const uint8_t *std_map, int rt_lo, int rt_hi, int rt_dst_base)
{
	if ((signo >= rt_lo) && (signo <= rt_hi)) {
		return signo - rt_lo + rt_dst_base;
	}

	if ((signo <= 0) || (signo > (int)Z_SIG_STD_MAP_MAX)) {
		return 0;
	}

	return std_map[signo];
}

static void sig_words_remap(const unsigned long *src, size_t src_nlongs, unsigned long *dst,
			    size_t dst_nlongs, int (*map)(int))
{
	memset(dst, 0, dst_nlongs * sizeof(unsigned long));

	for (size_t wi = 0; wi < src_nlongs; wi++) {
		unsigned long word = src[wi];

		while (word != 0) {
			const int bit = IS_ENABLED(CONFIG_64BIT)
						? u64_count_trailing_zeros(word)
						: u32_count_trailing_zeros(word);
			const int from = (int)(wi * BITS_PER_LONG + bit + 1);
			const int to = map(from);

			if (to > 0) {
				const int db = (to - 1) & BIT_MASK(LOG2(BITS_PER_LONG));
				const size_t di = (to - 1) / BITS_PER_LONG;

				if (IS_ENABLED(CONFIG_64BIT)) {
					dst[di] |= BIT64(db);
				} else {
					dst[di] |= BIT(db);
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
