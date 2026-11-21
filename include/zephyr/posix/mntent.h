/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief The mount table (<mntent.h>)
 *
 * Not part of POSIX: the mount table interface of System V and the GNU C
 * library, provided for the many programs that enumerate mounted file
 * systems through it. The table read is the kernel's list of mounted file
 * systems; the path given to setmntent() names no file and is ignored.
 *
 * @see <a href="https://man7.org/linux/man-pages/man3/getmntent.3.html">getmntent(3)</a>
 */

#ifndef ZEPHYR_INCLUDE_POSIX_MNTENT_H_
#define ZEPHYR_INCLUDE_POSIX_MNTENT_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief The conventional name of the table of mounted file systems. */
#define MOUNTED "/etc/mtab"
/** @brief The conventional name of the file system table. */
#define MNTTAB "/etc/fstab"

/** @brief One mounted file system. */
struct mntent {
	char *mnt_fsname; /**< Name of the mounted file system. */
	char *mnt_dir;    /**< Mount point. */
	char *mnt_type;   /**< File system type: "vfat", "ext2", "littlefs", "tmpfs", ... */
	char *mnt_opts;   /**< Mount options, comma-separated: "rw" or "ro". */
	int mnt_freq;     /**< Dump frequency, 0. */
	int mnt_passno;   /**< Pass number on parallel fsck, 0. */
};

/**
 * @brief Open the mount table for reading.
 *
 * @param filename Ignored: the table read is the kernel's.
 * @param type The fopen() mode; only reading is meaningful.
 * @return A stream for getmntent() and endmntent(), or NULL with errno set. The stream is
 *         not a stdio stream: it is for these functions alone.
 */
FILE *setmntent(const char *filename, const char *type);

/**
 * @brief Read the next entry of the mount table.
 *
 * @param stream A stream from setmntent().
 * @return The entry, valid until the next call on the stream or endmntent(), or NULL at the end
 *         of the table.
 */
struct mntent *getmntent(FILE *stream);

/**
 * @brief Close a mount table stream.
 *
 * @param stream A stream from setmntent().
 * @return 1.
 */
int endmntent(FILE *stream);

/**
 * @brief Find an option in an entry's mnt_opts.
 *
 * @param mnt The entry.
 * @param opt The option name, matched whole or as the name of a name=value option.
 * @return A pointer to the option within mnt_opts, or NULL.
 */
char *hasmntopt(const struct mntent *mnt, const char *opt);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_MNTENT_H_ */
