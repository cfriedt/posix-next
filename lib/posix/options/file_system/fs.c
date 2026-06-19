/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs_fs.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <zephyr/fs/fs.h>

int zvfs_fstat(int fd, struct stat *buf);

BUILD_ASSERT(PATH_MAX >= MAX_FILE_NAME, "PATH_MAX is less than MAX_FILE_NAME");

static struct fs_dirent fdirent;
static struct dirent pdirent;

/**
 * @brief Open a directory stream.
 *
 * See IEEE 1003.1
 */
DIR *opendir(const char *dirname)
{
	int rc;
	struct zvfs_fs_desc *ptr;

	ptr = zvfs_fs_desc_alloc(true);
	if (ptr == NULL) {
		errno = EMFILE;
		return NULL;
	}

	fs_dir_t_init(&ptr->dir);

	rc = fs_opendir(&ptr->dir, dirname);
	if (rc < 0) {
		zvfs_fs_desc_free(ptr);
		errno = -rc;
		return NULL;
	}

	return (DIR *)ptr;
}

/**
 * @brief Close a directory stream.
 *
 * See IEEE 1003.1
 */
int closedir(DIR *dirp)
{
	int rc;
	struct zvfs_fs_desc *ptr = (struct zvfs_fs_desc *)dirp;

	if (dirp == NULL) {
		errno = EBADF;
		return -1;
	}

	rc = fs_closedir(&ptr->dir);

	zvfs_fs_desc_free(ptr);

	if (rc < 0) {
		errno = -rc;
		return -1;
	}

	return 0;
}

/**
 * @brief Read a directory.
 *
 * See IEEE 1003.1
 */
struct dirent *readdir(DIR *dirp)
{
	int rc;
	struct zvfs_fs_desc *ptr = (struct zvfs_fs_desc *)dirp;

	if (dirp == NULL) {
		errno = EBADF;
		return NULL;
	}

	rc = fs_readdir(&ptr->dir, &fdirent);
	if (rc < 0) {
		errno = -rc;
		return NULL;
	}

	if (fdirent.name[0] == 0) {
		/* assume end-of-dir, leave errno untouched */
		return NULL;
	}

	rc = strlen(fdirent.name);
	rc = (rc < MAX_FILE_NAME) ? rc : (MAX_FILE_NAME - 1);
	(void)memcpy(pdirent.d_name, fdirent.name, rc);

	/* Make sure the name is NULL terminated */
	pdirent.d_name[rc] = '\0';
	return &pdirent;
}

/**
 * @brief Rename a file.
 *
 * See IEEE 1003.1
 */
int rename(const char *old, const char *new)
{
	int rc;

	rc = fs_rename(old, new);
	if (rc < 0) {
		errno = -rc;
		return -1;
	}

	return 0;
}

/**
 * @brief Remove a directory entry.
 *
 * See IEEE 1003.1
 */
int unlink(const char *path)
{
	int rc;

	rc = fs_unlink(path);
	if (rc < 0) {
		errno = -rc;
		return -1;
	}
	return 0;
}

/**
 * @brief Get file status.
 *
 * See IEEE 1003.1
 */
int stat(const char *path, struct stat *buf)
{
	int rc;
	struct fs_statvfs stat_vfs;
	struct fs_dirent stat_file;

	if (buf == NULL) {
		errno = EBADF;
		return -1;
	}

	rc = fs_statvfs(path, &stat_vfs);
	if (rc < 0) {
		errno = -rc;
		return -1;
	}

	rc = fs_stat(path, &stat_file);
	if (rc < 0) {
		errno = -rc;
		return -1;
	}

	memset(buf, 0, sizeof(struct stat));

	switch (stat_file.type) {
	case FS_DIR_ENTRY_FILE:
#if defined(_XOPEN_SOURCE)
		buf->st_mode = S_IFREG;
#endif
		break;
	case FS_DIR_ENTRY_DIR:
#if defined(_XOPEN_SOURCE)
		buf->st_mode = S_IFDIR;
#endif
		break;
	default:
		errno = EIO;
		return -1;
	}
	buf->st_size = stat_file.size;
#if defined(_XOPEN_SOURCE)
	buf->st_blksize = stat_vfs.f_bsize;
	/*
	 * This is a best effort guess, as this information is not provided
	 * by the fs_stat function.
	 */
	buf->st_blocks = (stat_file.size + stat_vfs.f_bsize - 1) / stat_vfs.f_bsize;
#endif

	return 0;
}

/**
 * @brief Make a directory.
 *
 * See IEEE 1003.1
 */
int mkdir(const char *path, mode_t mode)
{
	int rc;

	ARG_UNUSED(mode);

	rc = fs_mkdir(path);
	if (rc < 0) {
		errno = -rc;
		return -1;
	}

	return 0;
}

int fstat(int fildes, struct stat *buf)
{
	return zvfs_fstat(fildes, buf);
}
#ifdef CONFIG_POSIX_FILE_SYSTEM_ALIAS_FSTAT
FUNC_ALIAS(fstat, _fstat, int);
#endif

/**
 * @brief Remove a directory.
 *
 * See IEEE 1003.1
 */
int rmdir(const char *path)
{
	return unlink(path);
}
