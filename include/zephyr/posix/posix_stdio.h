/*
 * Copyright (c) 2025 The Zephyr Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief \<stdio.h\>: POSIX extensions to the C stdio library
 *
 * Provides POSIX and XSI extensions to the standard @c <stdio.h> interface
 * that are not part of ISO C. ISO C types, macros, and function prototypes
 * belong in the C library @c <stdio.h>; this header documents them only as
 * comments and declares the remaining POSIX interfaces.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdio.h.html">
 *      POSIX.1-2017 &lt;stdio.h&gt;</a>
 */

#ifndef ZEPHYR_INCLUDE_POSIX_POSIX_STDIO_H_
#define ZEPHYR_INCLUDE_POSIX_POSIX_STDIO_H_

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include <sys/types.h>
#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FILE must be defined in the libc stdio.h */
/* fpos_t must be defined in the libc stdio.h */
/* size_t must be defined in the libc stddef.h */

/* BUFSIZ must be defined in the libc stdio.h */
/* EOF must be defined in the libc stdio.h */
/* FOPEN_MAX must be defined in the libc stdio.h */
/* FILENAME_MAX must be defined in the libc stdio.h */
/* L_tmpnam must be defined in the libc stdio.h */
/* TMP_MAX must be defined in the libc stdio.h */

/* _IOFBF must be defined in the libc stdio.h */
/* _IOLBF must be defined in the libc stdio.h */
/* _IONBF must be defined in the libc stdio.h */

/* SEEK_CUR must be defined in the libc stdio.h */
/* SEEK_END must be defined in the libc stdio.h */
/* SEEK_SET must be defined in the libc stdio.h */

/* NULL must be defined in the libc stddef.h */

/* off_t must be defined in the libc sys/types.h */
/* ssize_t must be defined in the libc sys/types.h */

#if (_XOPEN_SOURCE >= 500) || defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Pathname of the directory for temporary files.
 * @ingroup posix_option_group_xsi_single_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdio.h.html
 */
#ifndef P_tmpdir
#define P_tmpdir "/tmp"
#endif

/**
 * @brief Maximum number of streams that one process can have open at one time.
 * @ingroup posix_option_group_xsi_single_process
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdio.h.html
 */
#if !defined(STREAM_MAX) && defined(FOPEN_MAX)
#define STREAM_MAX FOPEN_MAX
#endif
#endif /* (_XOPEN_SOURCE >= 500) || defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

/* stderr must be defined in the libc stdio.h */
/* stdin must be defined in the libc stdio.h */
/* stdout must be defined in the libc stdio.h */

/* clearerr() must be declared in the libc stdio.h */

#if (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__)
/**
 * @brief Generate a pathname for the controlling terminal.
 * @ingroup posix_option_group_xsi_single_process
 * @param s Optional buffer of at least @c L_ctermid bytes, or @c NULL.
 * @return Pathname of the controlling terminal, or @c NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/ctermid.html
 */
char *ctermid(char *s);
#endif /* (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__) */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Print formatted output to a file descriptor.
 * @ingroup posix_option_group_c_lib_ext
 * @param fildes Open file descriptor to write to.
 * @param format Format string.
 * @param ... Additional arguments for the format string.
 * @return Number of bytes written, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/dprintf.html
 */
int dprintf(int fildes, const char *ZRESTRICT format, ...);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

/* fclose() must be declared in the libc stdio.h */

#if (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__)
/**
 * @brief Associate a stream with an open file descriptor.
 * @ingroup posix_option_group_device_io
 * @param fd File descriptor to associate with the stream.
 * @param mode Mode string as for @c fopen().
 * @return Pointer to the stream, or @c NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fdopen.html
 */
FILE *fdopen(int fd, const char *mode);

/**
 * @brief Return the file descriptor associated with a stream.
 * @ingroup posix_option_group_device_io
 * @param stream Stream to query.
 * @return File descriptor associated with @p stream.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fileno.html
 */
int fileno(FILE *stream);
#endif /* (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__) */

/* feof() must be declared in the libc stdio.h */
/* ferror() must be declared in the libc stdio.h */
/* fflush() must be declared in the libc stdio.h */
/* fgetc() must be declared in the libc stdio.h */
/* fgetpos() must be declared in the libc stdio.h */
/* fgets() must be declared in the libc stdio.h */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Acquire a stdio stream lock.
 * @ingroup posix_option_group_file_locking
 * @param file Stream to lock.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/flockfile.html
 */
void flockfile(FILE *file);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Open a memory-backed stream.
 * @ingroup posix_option_group_c_lib_ext
 * @param buf Initial buffer, or @c NULL to allocate automatically.
 * @param size Size of @p buf when supplied by the caller.
 * @param mode Mode string as for @c fopen().
 * @return Pointer to the stream, or @c NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fmemopen.html
 */
FILE *fmemopen(void *ZRESTRICT buf, size_t size, const char *ZRESTRICT mode);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

/* fopen() must be declared in the libc stdio.h */
/* fprintf() must be declared in the libc stdio.h */
/* fputc() must be declared in the libc stdio.h */
/* fputs() must be declared in the libc stdio.h */
/* fread() must be declared in the libc stdio.h */
/* freopen() must be declared in the libc stdio.h */
/* fscanf() must be declared in the libc stdio.h */
/* fseek() must be declared in the libc stdio.h */

#if (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__)
/**
 * @brief Reposition a stream to a new offset.
 * @ingroup posix_option_group_fd_mgmt
 * @param stream Stream to reposition.
 * @param offset New file offset.
 * @param whence @c SEEK_SET, @c SEEK_CUR, or @c SEEK_END.
 * @return 0 on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/fseeko.html
 */
int fseeko(FILE *stream, off_t offset, int whence);
#endif /* (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__) */

/* fsetpos() must be declared in the libc stdio.h */
/* ftell() must be declared in the libc stdio.h */

#if (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__)
/**
 * @brief Obtain the current file offset of a stream.
 * @ingroup posix_option_group_fd_mgmt
 * @param stream Stream to query.
 * @return Current offset, or @c (off_t)-1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/ftello.html
 */
off_t ftello(FILE *stream);
#endif /* (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__) */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Try to acquire a stdio stream lock without blocking.
 * @ingroup posix_option_group_file_locking
 * @param file Stream to lock.
 * @return 0 if the lock was acquired, non-zero otherwise.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/ftrylockfile.html
 */
int ftrylockfile(FILE *file);

/**
 * @brief Release a stdio stream lock.
 * @ingroup posix_option_group_file_locking
 * @param file Stream to unlock.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/funlockfile.html
 */
void funlockfile(FILE *file);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

/* fwrite() must be declared in the libc stdio.h */
/* getc() must be declared in the libc stdio.h */
/* getchar() must be declared in the libc stdio.h */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Read a byte from a stream without acquiring the stream lock.
 * @ingroup posix_option_group_file_locking
 * @param stream Stream to read from.
 * @return Byte read as an @c unsigned char cast to @c int, or @c EOF.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getc_unlocked.html
 */
int getc_unlocked(FILE *stream);

/**
 * @brief Read a byte from @c stdin without acquiring the stream lock.
 * @ingroup posix_option_group_file_locking
 * @return Byte read as an @c unsigned char cast to @c int, or @c EOF.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getchar_unlocked.html
 */
int getchar_unlocked(void);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Read a delimited record from a stream.
 * @ingroup posix_option_group_c_lib_ext
 * @param lineptr Pointer to the buffer pointer; updated on return.
 * @param n Pointer to the buffer size; updated on return.
 * @param delimiter Record delimiter byte.
 * @param stream Stream to read from.
 * @return Number of bytes read, excluding the delimiter and NUL terminator,
 *         0 at end-of-file with no data read, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getdelim.html
 */
ssize_t getdelim(char **ZRESTRICT lineptr, size_t *ZRESTRICT n, int delimiter,
		 FILE *ZRESTRICT stream);

/**
 * @brief Read a line from a stream.
 * @ingroup posix_option_group_c_lib_ext
 * @param lineptr Pointer to the buffer pointer; updated on return.
 * @param n Pointer to the buffer size; updated on return.
 * @param stream Stream to read from.
 * @return Number of bytes read, excluding the newline and NUL terminator,
 *         0 at end-of-file with no data read, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getline.html
 */
ssize_t getline(char **ZRESTRICT lineptr, size_t *ZRESTRICT n, FILE *ZRESTRICT stream);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

/* gets() must be declared in the libc stdio.h */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Open a dynamic memory output stream.
 * @ingroup posix_option_group_c_lib_ext
 * @param bufp Pointer to the buffer pointer; updated on return.
 * @param sizep Pointer to the buffer size; updated on return.
 * @return Pointer to the stream, or @c NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/open_memstream.html
 */
FILE *open_memstream(char **bufp, size_t *sizep);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

#if (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__)
/**
 * @brief Close a pipe stream.
 * @ingroup posix_option_group_xsi_single_process
 * @param stream Stream returned by @c popen().
 * @return Termination status of the command, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pclose.html
 */
int pclose(FILE *stream);
#endif /* (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__) */

/* perror() must be declared in the libc stdio.h */

#if (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__)
/**
 * @brief Create a pipe to a process.
 * @ingroup posix_option_group_xsi_single_process
 * @param command Shell command to execute.
 * @param type @c "r" to read from the command, or @c "w" to write to it.
 * @return Pointer to the stream, or @c NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/popen.html
 */
FILE *popen(const char *command, const char *type);
#endif /* (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L) || defined(__DOXYGEN__) */

/* printf() must be declared in the libc stdio.h */
/* putc() must be declared in the libc stdio.h */
/* putchar() must be declared in the libc stdio.h */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Write a byte to a stream without acquiring the stream lock.
 * @ingroup posix_option_group_file_locking
 * @param c Byte to write.
 * @param stream Stream to write to.
 * @return Byte written as an @c unsigned char cast to @c int, or @c EOF.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/putc_unlocked.html
 */
int putc_unlocked(int c, FILE *stream);

/**
 * @brief Write a byte to @c stdout without acquiring the stream lock.
 * @ingroup posix_option_group_file_locking
 * @param c Byte to write.
 * @return Byte written as an @c unsigned char cast to @c int, or @c EOF.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/putchar_unlocked.html
 */
int putchar_unlocked(int c);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

/* puts() must be declared in the libc stdio.h */
/* remove() must be declared in the libc stdio.h */
/* rename() must be declared in the libc stdio.h */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Rename a file relative to directory file descriptors.
 * @ingroup posix_option_group_fd_mgmt
 * @param olddirfd File descriptor for the directory containing @p oldpath.
 * @param oldpath Existing pathname.
 * @param newdirfd File descriptor for the directory containing @p newpath.
 * @param newpath New pathname.
 * @return 0 on success, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/renameat.html
 */
int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

/* rewind() must be declared in the libc stdio.h */
/* scanf() must be declared in the libc stdio.h */
/* setbuf() must be declared in the libc stdio.h */
/* setvbuf() must be declared in the libc stdio.h */
/* snprintf() must be declared in the libc stdio.h */
/* sprintf() must be declared in the libc stdio.h */
/* sscanf() must be declared in the libc stdio.h */

#if (_XOPEN_SOURCE >= 500) || defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Generate a pathname for a temporary file (obsolescent).
 * @ingroup posix_option_group_xsi_single_process
 * @param dir Directory prefix, or @c NULL to use @c P_tmpdir.
 * @param pfx Filename prefix, or @c NULL.
 * @return Pointer to a pathname, or @c NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/tempnam.html
 */
char *tempnam(const char *dir, const char *pfx);
#endif /* (_XOPEN_SOURCE >= 500) || defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

/* tmpfile() must be declared in the libc stdio.h */
/* tmpnam() must be declared in the libc stdio.h */
/* ungetc() must be declared in the libc stdio.h */

#if (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__)
/**
 * @brief Print formatted output to a file descriptor.
 * @ingroup posix_option_group_c_lib_ext
 * @param fildes Open file descriptor to write to.
 * @param format Format string.
 * @param ap Arguments for the format string.
 * @return Number of bytes written, or -1 on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/vdprintf.html
 */
int vdprintf(int fildes, const char *ZRESTRICT format, va_list ap);
#endif /* (_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700) || defined(__DOXYGEN__) */

/* vfprintf() must be declared in the libc stdio.h */
/* vfscanf() must be declared in the libc stdio.h */
/* vprintf() must be declared in the libc stdio.h */
/* vscanf() must be declared in the libc stdio.h */
/* vsnprintf() must be declared in the libc stdio.h */
/* vsprintf() must be declared in the libc stdio.h */
/* vsscanf() must be declared in the libc stdio.h */

#ifdef __cplusplus
}
#endif

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#endif /* ZEPHYR_INCLUDE_POSIX_POSIX_STDIO_H_ */
