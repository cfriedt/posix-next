/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs.h>

static int fcntl_posix_to_zvfs_cmd(int cmd)
{
	switch (cmd) {
#if defined(F_DUPFD)
	case F_DUPFD:
		return ZVFS_F_DUPFD;
#endif
#if defined(F_DUPFD_CLOEXEC)
	case F_DUPFD_CLOEXEC:
		return ZVFS_F_DUPFD_CLOEXEC;
#endif
#if defined(F_GETFD)
	case F_GETFD:
		return ZVFS_F_GETFD;
#endif
#if defined(F_SETFD)
	case F_SETFD:
		return ZVFS_F_SETFD;
#endif
#if defined(F_GETFL)
	case F_GETFL:
		return ZVFS_F_GETFL;
#endif
#if defined(F_SETFL)
	case F_SETFL:
		return ZVFS_F_SETFL;
#endif
#if defined(F_GETLK)
	case F_GETLK:
		return ZVFS_F_GETLK;
#endif
#if defined(F_SETLK)
	case F_SETLK:
		return ZVFS_F_SETLK;
#endif
#if defined(F_SETLKW)
	case F_SETLKW:
		return ZVFS_F_SETLKW;
#endif
	default:
		return cmd;
	}
}

static int fcntl_posix_to_zvfs_arg(int zvfs_cmd, int arg)
{
	switch (zvfs_cmd) {
	case ZVFS_F_SETFD:
#if defined(FD_CLOEXEC) && FD_CLOEXEC != ZVFS_FD_CLOEXEC
		if (arg & FD_CLOEXEC) {
			arg = (arg & ~FD_CLOEXEC) | ZVFS_FD_CLOEXEC;
		}
#endif
		return arg;
	case ZVFS_F_SETFL:
#if defined(O_NONBLOCK) && O_NONBLOCK != ZVFS_O_NONBLOCK
		if (arg & O_NONBLOCK) {
			arg = (arg & ~O_NONBLOCK) | ZVFS_O_NONBLOCK;
		}
#endif
		return arg;
	default:
		return arg;
	}
}

static int fcntl_zvfs_to_posix_result(int zvfs_cmd, int res)
{
	switch (zvfs_cmd) {
	case ZVFS_F_GETFD:
#if defined(FD_CLOEXEC) && FD_CLOEXEC != ZVFS_FD_CLOEXEC
		if (res & ZVFS_FD_CLOEXEC) {
			res = (res & ~ZVFS_FD_CLOEXEC) | FD_CLOEXEC;
		}
#endif
		return res;
	case ZVFS_F_GETFL:
#if defined(O_NONBLOCK) && O_NONBLOCK != ZVFS_O_NONBLOCK
		if (res & ZVFS_O_NONBLOCK) {
			res = (res & ~ZVFS_O_NONBLOCK) | O_NONBLOCK;
		}
#endif
		return res;
	default:
		return res;
	}
}

static short fcntl_posix_to_zvfs_lock_type(short type)
{
	switch (type) {
#if defined(F_RDLCK)
	case F_RDLCK:
		return ZVFS_F_RDLCK;
#endif
#if defined(F_WRLCK)
	case F_WRLCK:
		return ZVFS_F_WRLCK;
#endif
#if defined(F_UNLCK)
	case F_UNLCK:
		return ZVFS_F_UNLCK;
#endif
	default:
		return type;
	}
}

static short fcntl_zvfs_to_posix_lock_type(short type)
{
	switch (type) {
	case ZVFS_F_RDLCK:
#if defined(F_RDLCK)
		return F_RDLCK;
#else
		return type;
#endif
	case ZVFS_F_WRLCK:
#if defined(F_WRLCK)
		return F_WRLCK;
#else
		return type;
#endif
	case ZVFS_F_UNLCK:
#if defined(F_UNLCK)
		return F_UNLCK;
#else
		return type;
#endif
	default:
		return type;
	}
}

static short fcntl_posix_to_zvfs_whence(short whence)
{
	switch (whence) {
#if defined(SEEK_SET)
	case SEEK_SET:
		return ZVFS_SEEK_SET;
#endif
#if defined(SEEK_CUR)
	case SEEK_CUR:
		return ZVFS_SEEK_CUR;
#endif
#if defined(SEEK_END)
	case SEEK_END:
		return ZVFS_SEEK_END;
#endif
	default:
		return whence;
	}
}

static short fcntl_zvfs_to_posix_whence(short whence)
{
	switch (whence) {
	case ZVFS_SEEK_SET:
#if defined(SEEK_SET)
		return SEEK_SET;
#else
		return whence;
#endif
	case ZVFS_SEEK_CUR:
#if defined(SEEK_CUR)
		return SEEK_CUR;
#else
		return whence;
#endif
	case ZVFS_SEEK_END:
#if defined(SEEK_END)
		return SEEK_END;
#else
		return whence;
#endif
	default:
		return whence;
	}
}

static void fcntl_posix_to_zvfs_flock(const struct flock *in, struct zvfs_flock *out)
{
	out->l_type = fcntl_posix_to_zvfs_lock_type(in->l_type);
	out->l_whence = fcntl_posix_to_zvfs_whence(in->l_whence);
	out->l_start = in->l_start;
	out->l_len = in->l_len;
	out->l_pid = in->l_pid;
}

static void fcntl_zvfs_to_posix_flock(const struct zvfs_flock *in, struct flock *out)
{
	out->l_type = fcntl_zvfs_to_posix_lock_type(in->l_type);
	out->l_whence = fcntl_zvfs_to_posix_whence(in->l_whence);
	out->l_start = in->l_start;
	out->l_len = in->l_len;
	out->l_pid = in->l_pid;
}

int fcntl(int fd, int cmd, ...)
{
	va_list args;
	int zvfs_cmd;
	int ret;

	zvfs_cmd = fcntl_posix_to_zvfs_cmd(cmd);

	va_start(args, cmd);

	switch (zvfs_cmd) {
	case ZVFS_F_GETLK:
	case ZVFS_F_SETLK:
	case ZVFS_F_SETLKW: {
		struct flock *posix_lock = va_arg(args, struct flock *);
		struct zvfs_flock zvfs_lock;

		va_end(args);

		if (posix_lock == NULL) {
			errno = EINVAL;
			return -1;
		}

		fcntl_posix_to_zvfs_flock(posix_lock, &zvfs_lock);
		ret = zvfs_fcntl(fd, zvfs_cmd, (uintptr_t)&zvfs_lock);
		if (ret < 0) {
			return -1;
		}

		if (zvfs_cmd == ZVFS_F_GETLK) {
			fcntl_zvfs_to_posix_flock(&zvfs_lock, posix_lock);
		}

		return ret;
	}
	default: {
		int arg = va_arg(args, int);

		va_end(args);

		return fcntl_zvfs_to_posix_result(zvfs_cmd,
						 zvfs_fcntl(fd, zvfs_cmd,
							    (uintptr_t)fcntl_posix_to_zvfs_arg(
								    zvfs_cmd, arg)));
	}
	}
}
#ifdef CONFIG_POSIX_FD_MGMT_ALIAS_FCNTL
FUNC_ALIAS(fcntl, _fcntl, int);
#endif /* CONFIG_POSIX_FD_MGMT_ALIAS_FCNTL */
