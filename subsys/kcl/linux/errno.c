/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * The C library numbers errno its own way; a Linux executable expects the
 * kernel's numbers. Every result leaves the layer through zkcl_linux_result().
 */

#include <errno.h>

#include <zephyr/sys/util.h>

#include "kcl_linux.h"

#define E(name, linux) [name] = (linux)

static const uint8_t linux_errno[] = {
	E(EPERM, 1), E(ENOENT, 2), E(ESRCH, 3), E(EINTR, 4), E(EIO, 5), E(ENXIO, 6),
	E(E2BIG, 7), E(ENOEXEC, 8), E(EBADF, 9), E(ECHILD, 10), E(EAGAIN, 11), E(ENOMEM, 12),
	E(EACCES, 13), E(EFAULT, 14), E(EBUSY, 16), E(EEXIST, 17), E(EXDEV, 18), E(ENODEV, 19),
	E(ENOTDIR, 20), E(EISDIR, 21), E(EINVAL, 22), E(ENFILE, 23), E(EMFILE, 24),
	E(ENOTTY, 25), E(EFBIG, 27), E(ENOSPC, 28), E(ESPIPE, 29), E(EROFS, 30), E(EMLINK, 31),
	E(EPIPE, 32), E(EDOM, 33), E(ERANGE, 34), E(EDEADLK, 35), E(ENAMETOOLONG, 36),
	E(ENOLCK, 37), E(ENOSYS, 38), E(ENOTEMPTY, 39), E(ELOOP, 40), E(ENOMSG, 42),
	E(EIDRM, 43),
#ifdef ENOSTR
	E(ENOSTR, 60),
#endif
#ifdef ENODATA
	E(ENODATA, 61),
#endif
#ifdef ETIME
	E(ETIME, 62),
#endif
#ifdef ENOSR
	E(ENOSR, 63),
#endif
#ifdef ENOLINK
	E(ENOLINK, 67),
#endif
	E(EPROTO, 71), E(EBADMSG, 74), E(EOVERFLOW, 75), E(EILSEQ, 84), E(ENOTSOCK, 88),
	E(EDESTADDRREQ, 89), E(EMSGSIZE, 90), E(EPROTOTYPE, 91), E(ENOPROTOOPT, 92),
	E(EPROTONOSUPPORT, 93),
#ifdef ESOCKTNOSUPPORT
	E(ESOCKTNOSUPPORT, 94),
#endif
	E(ENOTSUP, 95),
#if defined(EOPNOTSUPP) && (EOPNOTSUPP != ENOTSUP)
	E(EOPNOTSUPP, 95),
#endif
#ifdef EPFNOSUPPORT
	E(EPFNOSUPPORT, 96),
#endif
	E(EAFNOSUPPORT, 97), E(EADDRINUSE, 98), E(EADDRNOTAVAIL, 99), E(ENETDOWN, 100),
	E(ENETUNREACH, 101), E(ENETRESET, 102), E(ECONNABORTED, 103), E(ECONNRESET, 104),
	E(ENOBUFS, 105), E(EISCONN, 106), E(ENOTCONN, 107),
#ifdef ESHUTDOWN
	E(ESHUTDOWN, 108),
#endif
#ifdef ETOOMANYREFS
	E(ETOOMANYREFS, 109),
#endif
	E(ETIMEDOUT, 110), E(ECONNREFUSED, 111),
#ifdef EHOSTDOWN
	E(EHOSTDOWN, 112),
#endif
	E(EHOSTUNREACH, 113), E(EALREADY, 114), E(EINPROGRESS, 115), E(ECANCELED, 125),
#ifdef EOWNERDEAD
	E(EOWNERDEAD, 130),
#endif
#ifdef ENOTRECOVERABLE
	E(ENOTRECOVERABLE, 131),
#endif
};

int zkcl_linux_errno(int err)
{
	if ((err <= 0) || (err >= (int)ARRAY_SIZE(linux_errno)) || (linux_errno[err] == 0U)) {
		/* nothing closer: the executable sees an unknown failure */
		return 22;
	}

	return linux_errno[err];
}

long zkcl_linux_result(long ret)
{
	/* the Linux convention: [-4095, -1] is an error, anything else a value */
	if ((ret < 0) && (ret > -4096)) {
		return -zkcl_linux_errno((int)-ret);
	}

	return ret;
}
