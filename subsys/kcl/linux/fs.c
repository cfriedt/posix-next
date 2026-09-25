/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* descriptor and file system calls, over the virtual file system */

#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/process.h>
#include <zephyr/sys/zvfs.h>
#include <zephyr/sys/zvfs_fs.h>
#include <zephyr/zvfs/pipe.h>

#include "kcl_linux.h"

#define IOV_MAX 1024
#define PATH_MAX 256

#define LINUX_AT_FDCWD (-100)
#define LINUX_AT_SYMLINK_NOFOLLOW 0x100
#define LINUX_AT_EMPTY_PATH 0x1000

#define LINUX_O_ACCMODE 03
#define LINUX_O_CREAT 0100
#define LINUX_O_EXCL 0200
#define LINUX_O_TRUNC 01000
#define LINUX_O_APPEND 02000
#define LINUX_O_NONBLOCK 04000
#define LINUX_O_DIRECTORY 0200000
#define LINUX_O_CLOEXEC 02000000

#define LINUX_TCGETS 0x5401
#define LINUX_TCSETS 0x5402
#define LINUX_TCSETSW 0x5403
#define LINUX_TCSETSF 0x5404
#define LINUX_ICRNL 0000400
#define LINUX_ICANON 0000002
#define LINUX_ECHO 0000010
#define LINUX_TIOCGPGRP 0x540f
#define LINUX_TIOCSPGRP 0x5410
#define LINUX_TIOCGWINSZ 0x5413
#define LINUX_TIOCSWINSZ 0x5414
#define LINUX_FIONREAD 0x541b

/* the console: a terminal of 80 by 24, whose attributes the process keeps */
struct linux_winsize {
	uint16_t ws_row;
	uint16_t ws_col;
	uint16_t ws_xpixel;
	uint16_t ws_ypixel;
};

void zkcl_linux_termios_init(struct zkcl_linux_termios *t)
{
	static const uint8_t cc[19] = {
		[0] = 003, [1] = 034, [2] = 0177, [3] = 025, [4] = 004, [5] = 0, [6] = 1,
		[7] = 0, [8] = 021, [9] = 023, [10] = 032, [11] = 0, [12] = 022, [13] = 017,
		[14] = 027, [15] = 026, [16] = 0,
	};

	*t = (struct zkcl_linux_termios){
		.c_iflag = 0002400 | 0000400,	/* ICRNL | IXON */
		.c_oflag = 0000001 | 0000004,	/* OPOST | ONLCR */
		.c_cflag = 0000017 | 0000060 | 0000200, /* B38400 | CS8 | CREAD */
		.c_lflag = 0000001 | 0000002 | 0000010 | 0000020 | 0000040 | 0100000,
					/* ISIG | ICANON | ECHO | ECHOE | ECHOK | IEXTEN */
	};
	memcpy(t->c_cc, cc, sizeof(cc));
}

static int fd_ioctl(int fd, unsigned long request, ...)
{
	va_list args;
	int ret;

	va_start(args, request);
	ret = zvfs_ioctl(fd, request, args);
	va_end(args);

	return ret;
}

static bool is_console(unsigned int fd)
{
	return fd_ioctl(fd, ZFD_IOCTL_ISATTY) == 0;
}

static long result(long ret)
{
	return (ret < 0) ? -errno : ret;
}

static int dirfd_of(int linux_dirfd)
{
	return (linux_dirfd == LINUX_AT_FDCWD) ? ZVFS_AT_FDCWD : linux_dirfd;
}

static int open_flags(int linux_flags)
{
	static const struct {
		int linux;
		int zephyr;
	} bits[] = {
		{LINUX_O_CREAT, O_CREAT},	  {LINUX_O_EXCL, O_EXCL},
		{LINUX_O_TRUNC, O_TRUNC},	  {LINUX_O_APPEND, O_APPEND},
		{LINUX_O_NONBLOCK, O_NONBLOCK},	  {LINUX_O_DIRECTORY, O_DIRECTORY},
		{LINUX_O_CLOEXEC, O_CLOEXEC},
	};
	int flags;

	switch (linux_flags & LINUX_O_ACCMODE) {
	case 0:
		flags = O_RDONLY;
		break;
	case 1:
		flags = O_WRONLY;
		break;
	default:
		flags = O_RDWR;
		break;
	}
	for (size_t i = 0; i < ARRAY_SIZE(bits); i++) {
		if ((linux_flags & bits[i].linux) != 0) {
			flags |= bits[i].zephyr;
		}
	}

	return flags;
}

ZKCL_LINUX_IMPL(write)(unsigned int fd, const char *buf, size_t count)
{
	if (zkcl_linux_user_ok(buf, count, false) != 0) {
		return -EFAULT;
	}

	return result(zvfs_write(fd, buf, count));
}

ZKCL_LINUX_IMPL(read)(unsigned int fd, char *buf, size_t count)
{
	if (zkcl_linux_user_ok(buf, count, true) != 0) {
		return -EFAULT;
	}

	return result(zvfs_read(fd, buf, count));
}

static long rw_vectors(unsigned int fd, const struct zkcl_linux_iovec *vec, unsigned long vlen,
		       bool write)
{
	long total = 0;

	if ((vlen > IOV_MAX) || (zkcl_linux_user_ok(vec, vlen * sizeof(*vec), false) != 0)) {
		return (vlen > IOV_MAX) ? -EINVAL : -EFAULT;
	}
	for (unsigned long i = 0; i < vlen; i++) {
		void *base = vec[i].iov_base;
		size_t len = vec[i].iov_len;
		ssize_t n;

		if (zkcl_linux_user_ok(base, len, !write) != 0) {
			return (total > 0) ? total : -EFAULT;
		}
		n = write ? zvfs_write(fd, base, len) : zvfs_read(fd, base, len);
		if (n < 0) {
			return (total > 0) ? total : -errno;
		}
		total += n;
		if ((size_t)n < len) {
			break;
		}
	}

	return total;
}

ZKCL_LINUX_IMPL(writev)(unsigned long fd, const struct zkcl_linux_iovec *vec,
				      unsigned long vlen)
{
	return rw_vectors(fd, vec, vlen, true);
}

ZKCL_LINUX_IMPL(readv)(unsigned long fd, const struct zkcl_linux_iovec *vec,
				     unsigned long vlen)
{
	return rw_vectors(fd, vec, vlen, false);
}

ZKCL_LINUX_IMPL(close)(unsigned int fd)
{
	zkcl_linux_sock_forget(fd);

	return result(zvfs_close(fd));
}

static long openat_impl(int dfd, const char *filename, int flags,
				      zkcl_linux_umode_t mode)
{
	char path[PATH_MAX];
	int ret = zkcl_linux_user_string(path, filename, sizeof(path));

	if (ret != 0) {
		return ret;
	}

	ret = zvfs_openat(dirfd_of(dfd), path, open_flags(flags), mode);
	if ((ret >= 0) && ((flags & LINUX_O_CLOEXEC) != 0)) {
		/* the flag is the descriptor's: open() leaves it to fcntl() */
		(void)zvfs_fcntl(ret, ZVFS_F_SETFD, ZVFS_FD_CLOEXEC);
	}

	return result(ret);
}

ZKCL_LINUX_IMPL(openat)(int dfd, const char *filename, int flags,
				      zkcl_linux_umode_t mode)
{
	return openat_impl(dfd, filename, flags, mode);
}

ZKCL_LINUX_IMPL(open)(const char *filename, int flags, zkcl_linux_umode_t mode)
{
	return openat_impl(LINUX_AT_FDCWD, filename, flags, mode);
}

ZKCL_LINUX_IMPL(lseek)(unsigned int fd, zkcl_linux_off_t offset,
				     unsigned int whence)
{
	return result(zvfs_lseek(fd, offset, whence));
}

ZKCL_LINUX_IMPL(ioctl)(unsigned int fd, unsigned int cmd, unsigned long arg)
{
	struct zkcl_linux_process *p = zkcl_linux_current();
	void *uarg = (void *)arg;

	if (zvfs_fd_entry_get(fd) == NULL) {
		return -EBADF;
	}
	if ((p == NULL) || !is_console(fd)) {
		return -ENOTTY;
	}
	switch (cmd) {
	case LINUX_TCGETS:
		if (zkcl_linux_user_ok(uarg, sizeof(p->termios), true) != 0) {
			return -EFAULT;
		}
		memcpy(uarg, &p->termios, sizeof(p->termios));
		return 0;
	case LINUX_TCSETS:
	case LINUX_TCSETSW:
	case LINUX_TCSETSF: {
		unsigned int mode = 0;

		if (zkcl_linux_user_ok(uarg, sizeof(p->termios), false) != 0) {
			return -EFAULT;
		}
		memcpy(&p->termios, uarg, sizeof(p->termios));
		mode |= ((p->termios.c_lflag & LINUX_ICANON) != 0) ? ZVFS_CONSOLE_ICANON : 0;
		mode |= ((p->termios.c_lflag & LINUX_ECHO) != 0) ? ZVFS_CONSOLE_ECHO : 0;
		mode |= ((p->termios.c_iflag & LINUX_ICRNL) != 0) ? ZVFS_CONSOLE_ICRNL : 0;
		return result(fd_ioctl(fd, ZFD_IOCTL_CONSOLE_SET_MODE, mode));
	}
	case LINUX_TIOCGWINSZ: {
		struct linux_winsize ws = {.ws_row = 24, .ws_col = 80};

		if (zkcl_linux_user_ok(uarg, sizeof(ws), true) != 0) {
			return -EFAULT;
		}
		memcpy(uarg, &ws, sizeof(ws));
		return 0;
	}
	case LINUX_TIOCSWINSZ:
		return 0;
	case LINUX_TIOCGPGRP: {
		int pgrp = sys_pgrp_id(k_getpgid(k_getpid()));

		if (zkcl_linux_user_ok(uarg, sizeof(pgrp), true) != 0) {
			return -EFAULT;
		}
		memcpy(uarg, &pgrp, sizeof(pgrp));
		return 0;
	}
	case LINUX_TIOCSPGRP:
		return 0;
	default:
		return -ENOTTY;
	}
}

ZKCL_LINUX_IMPL(dup)(unsigned int fildes)
{
	return result(zvfs_dup(fildes, 0));
}

ZKCL_LINUX_IMPL(dup2)(unsigned int oldfd, unsigned int newfd)
{
	return result(zvfs_dup2(oldfd, newfd));
}

ZKCL_LINUX_IMPL(dup3)(unsigned int oldfd, unsigned int newfd, int flags)
{
	long ret;

	if (oldfd == newfd) {
		return -EINVAL;
	}
	ret = result(zvfs_dup2(oldfd, newfd));
	if ((ret >= 0) && ((flags & LINUX_O_CLOEXEC) != 0)) {
		(void)zvfs_fcntl(newfd, F_SETFD, FD_CLOEXEC);
	}

	return ret;
}

static void stat_convert(const struct zvfs_stat *zst, struct zkcl_linux_stat *st)
{
	memset(st, 0, sizeof(*st));
	st->st_dev = zst->dev;
	st->st_ino = zst->ino;
	st->st_mode = zst->mode;
	st->st_nlink = zst->nlink;
	st->st_size = zst->size;
	st->st_blksize = zst->blksize;
	st->st_blocks = zst->blocks;
	st->st_atime = zst->atime.tv_sec;
	st->st_atime_nsec = zst->atime.tv_nsec;
	st->st_mtime = zst->mtime.tv_sec;
	st->st_mtime_nsec = zst->mtime.tv_nsec;
	st->st_ctime = zst->ctime.tv_sec;
	st->st_ctime_nsec = zst->ctime.tv_nsec;
}

static long fstat_into(unsigned int fd, struct zkcl_linux_stat *statbuf)
{
	struct zvfs_stat zst;
	struct zkcl_linux_stat st;

	if (zkcl_linux_user_ok(statbuf, sizeof(*statbuf), true) != 0) {
		return -EFAULT;
	}
	if (zvfs_fstat(fd, &zst) < 0) {
		return -errno;
	}
	stat_convert(&zst, &st);
	memcpy(statbuf, &st, sizeof(st));

	return 0;
}

ZKCL_LINUX_IMPL(fstat)(unsigned int fd, struct zkcl_linux_stat *statbuf)
{
	return fstat_into(fd, statbuf);
}

static long newfstatat_impl(int dfd, const char *filename,
					  struct zkcl_linux_stat *statbuf, int flag)
{
	char path[PATH_MAX];
	struct zvfs_stat zst;
	struct zkcl_linux_stat st;
	int zflags = 0;
	int ret;

	ret = zkcl_linux_user_string(path, filename, sizeof(path));
	if (ret != 0) {
		return ret;
	}
	if ((path[0] == '\0') && ((flag & LINUX_AT_EMPTY_PATH) != 0)) {
		return fstat_into(dfd, statbuf);
	}
	if (zkcl_linux_user_ok(statbuf, sizeof(*statbuf), true) != 0) {
		return -EFAULT;
	}
	if ((flag & LINUX_AT_SYMLINK_NOFOLLOW) != 0) {
		zflags |= ZVFS_AT_SYMLINK_NOFOLLOW;
	}
	if (zvfs_statat(dirfd_of(dfd), path, &zst, zflags) < 0) {
		return -errno;
	}
	stat_convert(&zst, &st);
	memcpy(statbuf, &st, sizeof(st));

	return 0;
}

ZKCL_LINUX_IMPL(newfstatat)(int dfd, const char *filename,
					  struct zkcl_linux_stat *statbuf, int flag)
{
	return newfstatat_impl(dfd, filename, statbuf, flag);
}

static long readlinkat_impl(int dfd, const char *pathname, char *buf, int bufsiz)
{
	char path[PATH_MAX];
	int ret = zkcl_linux_user_string(path, pathname, sizeof(path));

	if (ret != 0) {
		return ret;
	}
	if ((bufsiz <= 0) || (zkcl_linux_user_ok(buf, bufsiz, true) != 0)) {
		return (bufsiz <= 0) ? -EINVAL : -EFAULT;
	}

	return result(zvfs_readlinkat(dirfd_of(dfd), path, buf, bufsiz));
}

ZKCL_LINUX_IMPL(readlinkat)(int dfd, const char *pathname, char *buf, int bufsiz)
{
	return readlinkat_impl(dfd, pathname, buf, bufsiz);
}

ZKCL_LINUX_IMPL(fsync)(unsigned int fd)
{
	return result(zvfs_fsync(fd));
}

ZKCL_LINUX_IMPL(ftruncate)(unsigned int fd, zkcl_linux_off_t length)
{
	return result(zvfs_ftruncate(fd, length));
}

static long mkdirat_impl(int dfd, const char *pathname, zkcl_linux_umode_t mode)
{
	char path[PATH_MAX];
	int ret = zkcl_linux_user_string(path, pathname, sizeof(path));

	if (ret != 0) {
		return ret;
	}

	return result(zvfs_mkdirat(dirfd_of(dfd), path, mode));
}

ZKCL_LINUX_IMPL(mkdirat)(int dfd, const char *pathname, zkcl_linux_umode_t mode)
{
	return mkdirat_impl(dfd, pathname, mode);
}

static long mknodat_impl(int dfd, const char *filename, zkcl_linux_umode_t mode,
					unsigned int dev)
{
	char path[PATH_MAX];
	int ret = zkcl_linux_user_string(path, filename, sizeof(path));

	if (ret != 0) {
		return ret;
	}
	switch (mode & ZVFS_MODE_IFMT) {
	case 0:
	case ZVFS_MODE_IFREG:
	case ZVFS_MODE_IFIFO:
		/* the type and permission bits are the kernel's own */
		return result(zvfs_mknodat(dirfd_of(dfd), path, mode, dev));
	default:
		/* no device or socket nodes exist to create */
		return -EPERM;
	}
}

ZKCL_LINUX_IMPL(mknodat)(int dfd, const char *filename, zkcl_linux_umode_t mode,
					unsigned int dev)
{
	return mknodat_impl(dfd, filename, mode, dev);
}

ZKCL_LINUX_IMPL(mknod)(const char *filename, zkcl_linux_umode_t mode, unsigned int dev)
{
	return mknodat_impl(LINUX_AT_FDCWD, filename, mode, dev);
}

static long unlinkat_impl(int dfd, const char *pathname, int flag)
{
	char path[PATH_MAX];
	int ret = zkcl_linux_user_string(path, pathname, sizeof(path));

	if (ret != 0) {
		return ret;
	}

	return result(zvfs_unlinkat(dirfd_of(dfd), path, (flag & 0x200) ? ZVFS_AT_REMOVEDIR : 0));
}

ZKCL_LINUX_IMPL(unlinkat)(int dfd, const char *pathname, int flag)
{
	return unlinkat_impl(dfd, pathname, flag);
}

ZKCL_LINUX_IMPL(getcwd)(char *buf, unsigned long size)
{
	long ret;

	if (zkcl_linux_user_ok(buf, size, true) != 0) {
		return -EFAULT;
	}
	ret = result(zvfs_getcwd(buf, size));

	return (ret < 0) ? ret : (long)(strlen(buf) + 1);
}

ZKCL_LINUX_IMPL(chdir)(const char *filename)
{
	char path[PATH_MAX];
	int ret = zkcl_linux_user_string(path, filename, sizeof(path));

	if (ret != 0) {
		return ret;
	}

	return result(zvfs_chdir(path));
}

/* the classic path calls of the architectures that still number them */

ZKCL_LINUX_IMPL(mkdir)(const char *pathname, zkcl_linux_umode_t mode)
{
	return mkdirat_impl(LINUX_AT_FDCWD, pathname, mode);
}

ZKCL_LINUX_IMPL(rmdir)(const char *pathname)
{
	return unlinkat_impl(LINUX_AT_FDCWD, pathname, 0x200);
}

ZKCL_LINUX_IMPL(unlink)(const char *pathname)
{
	return unlinkat_impl(LINUX_AT_FDCWD, pathname, 0);
}

ZKCL_LINUX_IMPL(stat)(const char *filename, struct zkcl_linux_stat *statbuf)
{
	return newfstatat_impl(LINUX_AT_FDCWD, filename, statbuf, 0);
}

ZKCL_LINUX_IMPL(lstat)(const char *filename, struct zkcl_linux_stat *statbuf)
{
	return newfstatat_impl(LINUX_AT_FDCWD, filename, statbuf,
						    LINUX_AT_SYMLINK_NOFOLLOW);
}

ZKCL_LINUX_IMPL(readlink)(const char *path, char *buf, int bufsiz)
{
	return readlinkat_impl(LINUX_AT_FDCWD, path, buf, bufsiz);
}

static long faccessat_impl(int dfd, const char *filename, int mode)
{
	char path[PATH_MAX];
	int ret = zkcl_linux_user_string(path, filename, sizeof(path));

	if (ret != 0) {
		return ret;
	}

	return result(zvfs_accessat(dirfd_of(dfd), path, mode, 0));
}

ZKCL_LINUX_IMPL(faccessat)(int dfd, const char *filename, int mode)
{
	return faccessat_impl(dfd, filename, mode);
}

ZKCL_LINUX_IMPL(faccessat2)(int dfd, const char *filename, int mode, int flags)
{
	ARG_UNUSED(flags);

	return faccessat_impl(dfd, filename, mode);
}

ZKCL_LINUX_IMPL(access)(const char *filename, int mode)
{
	return faccessat_impl(LINUX_AT_FDCWD, filename, mode);
}

static long renameat_impl(int olddfd, const char *oldname, int newdfd,
					const char *newname)
{
	char oldpath[PATH_MAX];
	char newpath[PATH_MAX];
	int ret = zkcl_linux_user_string(oldpath, oldname, sizeof(oldpath));

	if (ret == 0) {
		ret = zkcl_linux_user_string(newpath, newname, sizeof(newpath));
	}
	if (ret != 0) {
		return ret;
	}

	return result(zvfs_renameat(dirfd_of(olddfd), oldpath, dirfd_of(newdfd), newpath));
}

ZKCL_LINUX_IMPL(renameat)(int olddfd, const char *oldname, int newdfd,
					const char *newname)
{
	return renameat_impl(olddfd, oldname, newdfd, newname);
}

ZKCL_LINUX_IMPL(renameat2)(int olddfd, const char *oldname, int newdfd,
					 const char *newname, unsigned int flags)
{
	if (flags != 0U) {
		return -EINVAL;
	}

	return renameat_impl(olddfd, oldname, newdfd, newname);
}

ZKCL_LINUX_IMPL(rename)(const char *oldname, const char *newname)
{
	return renameat_impl(LINUX_AT_FDCWD, oldname, LINUX_AT_FDCWD,
						  newname);
}

static long symlinkat_impl(const char *oldname, int newdfd, const char *newname)
{
	char target[PATH_MAX];
	char path[PATH_MAX];
	int ret = zkcl_linux_user_string(target, oldname, sizeof(target));

	if (ret == 0) {
		ret = zkcl_linux_user_string(path, newname, sizeof(path));
	}
	if (ret != 0) {
		return ret;
	}

	return result(zvfs_symlinkat(target, dirfd_of(newdfd), path));
}

ZKCL_LINUX_IMPL(symlinkat)(const char *oldname, int newdfd, const char *newname)
{
	return symlinkat_impl(oldname, newdfd, newname);
}

ZKCL_LINUX_IMPL(symlink)(const char *oldname, const char *newname)
{
	return symlinkat_impl(oldname, LINUX_AT_FDCWD, newname);
}

ZKCL_LINUX_IMPL(truncate)(const char *path, zkcl_linux_off_t length)
{
	char kpath[PATH_MAX];
	int ret = zkcl_linux_user_string(kpath, path, sizeof(kpath));

	if (ret != 0) {
		return ret;
	}

	return result(zvfs_truncate(kpath, length));
}

#define LINUX_F_DUPFD 0
#define LINUX_F_GETFD 1
#define LINUX_F_SETFD 2
#define LINUX_F_GETFL 3
#define LINUX_F_SETFL 4
#define LINUX_F_DUPFD_CLOEXEC 1030
#define LINUX_FD_CLOEXEC 1

/* the status flags, in the executable's numbering */
static long status_flags_to_linux(int flags)
{
	long out = flags & O_ACCMODE;

	if ((flags & O_APPEND) != 0) {
		out |= LINUX_O_APPEND;
	}
	if ((flags & O_NONBLOCK) != 0) {
		out |= LINUX_O_NONBLOCK;
	}

	return out;
}

ZKCL_LINUX_IMPL(fcntl)(unsigned int fd, unsigned int cmd, unsigned long arg)
{
	long ret;

	switch (cmd) {
	case LINUX_F_DUPFD:
		return result(zvfs_fcntl(fd, ZVFS_F_DUPFD, arg));
	case LINUX_F_DUPFD_CLOEXEC:
		return result(zvfs_fcntl(fd, ZVFS_F_DUPFD_CLOEXEC, arg));
	case LINUX_F_GETFD:
		ret = result(zvfs_fcntl(fd, ZVFS_F_GETFD, 0));
		return (ret < 0) ? ret : ((ret & ZVFS_FD_CLOEXEC) ? LINUX_FD_CLOEXEC : 0);
	case LINUX_F_SETFD:
		return result(zvfs_fcntl(fd, ZVFS_F_SETFD,
					 ((arg & LINUX_FD_CLOEXEC) != 0UL) ? ZVFS_FD_CLOEXEC : 0));
	case LINUX_F_GETFL:
		ret = result(zvfs_fcntl(fd, ZVFS_F_GETFL, 0));
		return (ret < 0) ? ret : status_flags_to_linux((int)ret);
	case LINUX_F_SETFL:
		return result(zvfs_fcntl(fd, ZVFS_F_SETFL, open_flags((int)arg) &
							   (O_APPEND | O_NONBLOCK)));
	default:
		return -EINVAL;
	}
}

static long pipe2_impl(int *fildes, int flags)
{
	int fds[2];
	int zflags = 0;
	int ret;

	if (zkcl_linux_user_ok(fildes, sizeof(fds), true) != 0) {
		return -EFAULT;
	}
	if ((flags & LINUX_O_CLOEXEC) != 0) {
		zflags |= O_CLOEXEC;
	}
	if ((flags & LINUX_O_NONBLOCK) != 0) {
		zflags |= O_NONBLOCK;
	}
	ret = zvfs_pipe(fds, zflags);
	if (ret < 0) {
		return -errno;
	}
	if ((flags & LINUX_O_CLOEXEC) != 0) {
		(void)zvfs_fcntl(fds[0], ZVFS_F_SETFD, ZVFS_FD_CLOEXEC);
		(void)zvfs_fcntl(fds[1], ZVFS_F_SETFD, ZVFS_FD_CLOEXEC);
	}
	memcpy(fildes, fds, sizeof(fds));

	return 0;
}

ZKCL_LINUX_IMPL(pipe2)(int *fildes, int flags)
{
	return pipe2_impl(fildes, flags);
}

ZKCL_LINUX_IMPL(pipe)(int *fildes)
{
	return pipe2_impl(fildes, 0);
}

/* a directory entry as getdents64() returns it */
struct linux_dirent64 {
	uint64_t d_ino;
	int64_t d_off;
	uint16_t d_reclen;
	uint8_t d_type;
	char d_name[];
};

#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_DIR 4
#define DT_REG 8
#define DT_LNK 10

static uint8_t dirent_type(uint32_t mode)
{
	switch (mode & ZVFS_MODE_IFMT) {
	case ZVFS_MODE_IFDIR:
		return DT_DIR;
	case ZVFS_MODE_IFREG:
		return DT_REG;
	case ZVFS_MODE_IFLNK:
		return DT_LNK;
	case ZVFS_MODE_IFIFO:
		return DT_FIFO;
	default:
		return DT_UNKNOWN;
	}
}

ZKCL_LINUX_IMPL(getdents64)(unsigned int fd, struct zkcl_linux_linux_dirent64 *dirent,
					  unsigned int count)
{
	struct zkcl_linux_process *p = zkcl_linux_current();
	uint8_t *out = (uint8_t *)dirent;
	unsigned int used = 0;
	int64_t off = 0;

	if (p == NULL) {
		return -EINVAL;
	}
	if (zkcl_linux_user_ok(dirent, count, true) != 0) {
		return -EFAULT;
	}
	if (zvfs_fdopendir(fd) == NULL) {
		return -errno;
	}
	while (true) {
		struct zvfs_dirent *ent = &p->dirent;
		struct linux_dirent64 *d;
		size_t namelen;
		size_t reclen;
		int ret;

		if (!(p->dirent_pending && (p->dirent_fd == (int)fd))) {
			ret = zvfs_readdir(fd, ent);
			if (ret < 0) {
				return (used > 0) ? (long)used : -errno;
			}
			if (ret > 0) {
				break;
			}
		}
		p->dirent_pending = false;
		namelen = strlen(ent->d_name);
		reclen = ROUND_UP(offsetof(struct linux_dirent64, d_name) + namelen + 1, 8);
		if (used + reclen > count) {
			/* keep it for the next call */
			p->dirent_fd = fd;
			p->dirent_pending = true;
			if (used == 0) {
				return -EINVAL;
			}
			break;
		}
		d = (struct linux_dirent64 *)(out + used);
		d->d_ino = (ent->d_ino != 0UL) ? ent->d_ino : 1;
		d->d_off = ++off;
		d->d_reclen = reclen;
		d->d_type = dirent_type(ent->d_type);
		memcpy(d->d_name, ent->d_name, namelen + 1);
		used += reclen;
	}

	return used;
}
