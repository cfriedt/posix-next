/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * The symbol table the busybox extension links against at load time: every
 * libc and POSIX symbol busybox references must be exported by the image.
 */

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <glob.h>
#include <poll.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <termios.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <zephyr/getopt.h>
#include <zephyr/llext/symbol.h>

/* errno may be thread-local; extensions resolve it per call through this */
int *__errno_location(void)
{
	return &errno;
}
EXPORT_SYMBOL(__errno_location);

EXPORT_SYMBOL(access);
EXPORT_SYMBOL(atoi);
EXPORT_SYMBOL(chdir);
EXPORT_SYMBOL(chmod);
EXPORT_SYMBOL(close);
EXPORT_SYMBOL(closedir);
EXPORT_SYMBOL(dup2);
EXPORT_SYMBOL(_exit);
EXPORT_SYMBOL(exit);
EXPORT_SYMBOL(fchdir);
EXPORT_SYMBOL(fclose);
EXPORT_SYMBOL(fcntl);
EXPORT_SYMBOL(fdopen);
EXPORT_SYMBOL(fflush);
EXPORT_SYMBOL(fgetc);
EXPORT_SYMBOL(fileno);
EXPORT_SYMBOL(fopen);
EXPORT_SYMBOL(fork);
EXPORT_SYMBOL(fprintf);
EXPORT_SYMBOL(fputc);
EXPORT_SYMBOL(fputs);
EXPORT_SYMBOL(free);
EXPORT_SYMBOL(fstat);
EXPORT_SYMBOL(getcwd);
EXPORT_SYMBOL(getegid);
EXPORT_SYMBOL(getenv);
EXPORT_SYMBOL(geteuid);
EXPORT_SYMBOL(getpid);
EXPORT_SYMBOL(gettimeofday);
EXPORT_SYMBOL(ioctl);
EXPORT_SYMBOL(link);
EXPORT_SYMBOL(localtime);
EXPORT_SYMBOL(lseek);
EXPORT_SYMBOL(lstat);
EXPORT_SYMBOL(malloc);
EXPORT_SYMBOL(memcpy);
EXPORT_SYMBOL(memmove);
EXPORT_SYMBOL(memset);
EXPORT_SYMBOL(mkdir);
EXPORT_SYMBOL(mkstemp);
EXPORT_SYMBOL(mktime);
EXPORT_SYMBOL(mmap);
EXPORT_SYMBOL(open);
EXPORT_SYMBOL(opendir);
EXPORT_SYMBOL(pipe);
EXPORT_SYMBOL(printf);
EXPORT_SYMBOL(puts);
EXPORT_SYMBOL(rand);
EXPORT_SYMBOL(read);
EXPORT_SYMBOL(readdir);
EXPORT_SYMBOL(readlink);
EXPORT_SYMBOL(realloc);
EXPORT_SYMBOL(rename);
EXPORT_SYMBOL(rmdir);
EXPORT_SYMBOL(setenv);
EXPORT_SYMBOL(sprintf);
EXPORT_SYMBOL(srand);
EXPORT_SYMBOL(sscanf);
EXPORT_SYMBOL(stat);
EXPORT_SYMBOL(stderr);
EXPORT_SYMBOL(stdin);
EXPORT_SYMBOL(stdout);
EXPORT_SYMBOL(strchr);
EXPORT_SYMBOL(strcmp);
EXPORT_SYMBOL(strcpy);
EXPORT_SYMBOL(strdup);
EXPORT_SYMBOL(strerror);
EXPORT_SYMBOL(strftime);
EXPORT_SYMBOL(strlen);
EXPORT_SYMBOL(strncasecmp);
EXPORT_SYMBOL(strncmp);
EXPORT_SYMBOL(strncpy);
EXPORT_SYMBOL(strndup);
EXPORT_SYMBOL(strpbrk);
EXPORT_SYMBOL(strrchr);
EXPORT_SYMBOL(strtol);
EXPORT_SYMBOL(strtoll);
EXPORT_SYMBOL(strtoul);
EXPORT_SYMBOL(strtoull);
EXPORT_SYMBOL(symlink);
EXPORT_SYMBOL(time);
EXPORT_SYMBOL(umask);
EXPORT_SYMBOL(uname);
EXPORT_SYMBOL(unlink);
EXPORT_SYMBOL(unsetenv);
EXPORT_SYMBOL(vsnprintf);
EXPORT_SYMBOL(waitpid);
EXPORT_SYMBOL(write);

/* getuid()/getgid() and friends come from POSIX_USER_GROUPS */
EXPORT_SYMBOL(getgid);
EXPORT_SYMBOL(getuid);
EXPORT_SYMBOL(setegid);
EXPORT_SYMBOL(seteuid);
EXPORT_SYMBOL(setgid);
EXPORT_SYMBOL(setuid);

/* busybox keeps its own option-parsing state via the reentrant getopt */
EXPORT_SYMBOL(zephyr_getopt);

/* the terminal: POSIX_DEVICE_SPECIFIC over the console */
EXPORT_SYMBOL(isatty);
EXPORT_SYMBOL(tcgetattr);
EXPORT_SYMBOL(tcsetattr);

/* the shell: pattern matching, signals, process control */
extern char **environ;
EXPORT_SYMBOL(environ);
EXPORT_SYMBOL(execvp);
EXPORT_SYMBOL(fnmatch);
EXPORT_SYMBOL(getc);
EXPORT_SYMBOL(getppid);
EXPORT_SYMBOL(glob);
EXPORT_SYMBOL(globfree);
EXPORT_SYMBOL(poll);
EXPORT_SYMBOL(putenv);
EXPORT_SYMBOL(raise);
EXPORT_SYMBOL(sigaction);
EXPORT_SYMBOL(sigaddset);
EXPORT_SYMBOL(sigdelset);
EXPORT_SYMBOL(sigismember);
EXPORT_SYMBOL(signal);
EXPORT_SYMBOL(sigprocmask);
EXPORT_SYMBOL(sigsuspend);
EXPORT_SYMBOL(strcspn);
EXPORT_SYMBOL(strspn);
EXPORT_SYMBOL(wait);
