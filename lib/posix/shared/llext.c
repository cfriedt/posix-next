/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * The POSIX surface, exported to linkable loadable extensions: a conformant
 * application built as an extension resolves these at load time. Every
 * export is gated by the Kconfig option of the Option Group (or Option)
 * that provides the function; the tables in doc/posix are the source of
 * truth, and each section lists the pages it came from.
 */

#include <aio.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <glob.h>
#include <grp.h>
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <monetary.h>
#include <mqueue.h>
#include <net/if.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <pwd.h>
#include <regex.h>
#include <sched.h>
#include <semaphore.h>
#include <setjmp.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stropts.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <syslog.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#include <zephyr/getopt.h>
#include <zephyr/llext/symbol.h>
#include <zephyr/posix/posix_setjmp.h>

/* errno may be thread-local; extensions resolve it per call through this */
int *__errno_location(void)
{
	return &errno;
}
EXPORT_SYMBOL(__errno_location);


/* BARRIERS */
#ifdef CONFIG_POSIX_BARRIERS
EXPORT_SYMBOL(pthread_barrier_destroy);
EXPORT_SYMBOL(pthread_barrier_init);
EXPORT_SYMBOL(pthread_barrier_wait);
EXPORT_SYMBOL(pthread_barrierattr_destroy);
EXPORT_SYMBOL(pthread_barrierattr_getpshared);
EXPORT_SYMBOL(pthread_barrierattr_init);
EXPORT_SYMBOL(pthread_barrierattr_setpshared);
#endif /* CONFIG_POSIX_BARRIERS */

/* C_LANG_JUMP: provided by the C library for every configuration */
EXPORT_SYMBOL(longjmp);
EXPORT_SYMBOL(setjmp);

/* C_LANG_SUPPORT_R */
#ifdef CONFIG_POSIX_C_LANG_SUPPORT_R
EXPORT_SYMBOL(asctime_r);
EXPORT_SYMBOL(ctime_r);
EXPORT_SYMBOL(gmtime_r);
EXPORT_SYMBOL(localtime_r);
EXPORT_SYMBOL(rand_r);
EXPORT_SYMBOL(strerror_r);
EXPORT_SYMBOL(strtok_r);
#endif /* CONFIG_POSIX_C_LANG_SUPPORT_R */

/* C_LIB_EXT */
#ifdef CONFIG_POSIX_C_LIB_EXT
EXPORT_SYMBOL(fnmatch);
EXPORT_SYMBOL(getopt);
EXPORT_SYMBOL(getsubopt);
EXPORT_SYMBOL(stpcpy);
EXPORT_SYMBOL(stpncpy);
EXPORT_SYMBOL(strcasecmp);
EXPORT_SYMBOL(strdup);
EXPORT_SYMBOL(strfmon);
EXPORT_SYMBOL(strncasecmp);
EXPORT_SYMBOL(strndup);
EXPORT_SYMBOL(strnlen);
#endif /* CONFIG_POSIX_C_LIB_EXT */

/* CLOCK_SELECTION */
#ifdef CONFIG_POSIX_CLOCK_SELECTION
EXPORT_SYMBOL(clock_nanosleep);
EXPORT_SYMBOL(pthread_condattr_getclock);
EXPORT_SYMBOL(pthread_condattr_setclock);
#endif /* CONFIG_POSIX_CLOCK_SELECTION */

/* DEVICE_IO */
#ifdef CONFIG_POSIX_DEVICE_IO
EXPORT_SYMBOL(clearerr);
EXPORT_SYMBOL(close);
EXPORT_SYMBOL(fclose);
EXPORT_SYMBOL(fdopen);
EXPORT_SYMBOL(feof);
EXPORT_SYMBOL(ferror);
EXPORT_SYMBOL(fflush);
EXPORT_SYMBOL(fgetc);
EXPORT_SYMBOL(fgets);
EXPORT_SYMBOL(fileno);
EXPORT_SYMBOL(fopen);
EXPORT_SYMBOL(fprintf);
EXPORT_SYMBOL(fputc);
EXPORT_SYMBOL(fputs);
EXPORT_SYMBOL(fread);
EXPORT_SYMBOL(freopen);
EXPORT_SYMBOL(fscanf);
EXPORT_SYMBOL(fwrite);
EXPORT_SYMBOL(getc);
EXPORT_SYMBOL(getchar);
EXPORT_SYMBOL(open);
EXPORT_SYMBOL(perror);
EXPORT_SYMBOL(poll);
EXPORT_SYMBOL(pread);
EXPORT_SYMBOL(printf);
EXPORT_SYMBOL(pselect);
EXPORT_SYMBOL(putc);
EXPORT_SYMBOL(putchar);
EXPORT_SYMBOL(puts);
EXPORT_SYMBOL(pwrite);
EXPORT_SYMBOL(read);
EXPORT_SYMBOL(scanf);
EXPORT_SYMBOL(select);
EXPORT_SYMBOL(setbuf);
EXPORT_SYMBOL(setvbuf);
EXPORT_SYMBOL(ungetc);
EXPORT_SYMBOL(vfprintf);
EXPORT_SYMBOL(vfscanf);
EXPORT_SYMBOL(vprintf);
EXPORT_SYMBOL(vscanf);
EXPORT_SYMBOL(write);
#endif /* CONFIG_POSIX_DEVICE_IO */

/* DEVICE_SPECIFIC */
#ifdef CONFIG_POSIX_DEVICE_SPECIFIC
EXPORT_SYMBOL(cfgetispeed);
EXPORT_SYMBOL(cfgetospeed);
EXPORT_SYMBOL(cfsetispeed);
EXPORT_SYMBOL(cfsetospeed);
EXPORT_SYMBOL(ctermid);
EXPORT_SYMBOL(isatty);
EXPORT_SYMBOL(tcdrain);
EXPORT_SYMBOL(tcflow);
EXPORT_SYMBOL(tcflush);
EXPORT_SYMBOL(tcgetattr);
EXPORT_SYMBOL(tcsendbreak);
EXPORT_SYMBOL(tcsetattr);
EXPORT_SYMBOL(ttyname);
#endif /* CONFIG_POSIX_DEVICE_SPECIFIC */

/* DEVICE_SPECIFIC_R */
#ifdef CONFIG_POSIX_DEVICE_SPECIFIC_R
EXPORT_SYMBOL(ttyname_r);
#endif /* CONFIG_POSIX_DEVICE_SPECIFIC_R */

/* FD_MGMT */
#ifdef CONFIG_POSIX_FD_MGMT
EXPORT_SYMBOL(dup);
EXPORT_SYMBOL(dup2);
EXPORT_SYMBOL(fcntl);
EXPORT_SYMBOL(fgetpos);
EXPORT_SYMBOL(fseek);
EXPORT_SYMBOL(fseeko);
EXPORT_SYMBOL(fsetpos);
EXPORT_SYMBOL(ftell);
EXPORT_SYMBOL(ftello);
EXPORT_SYMBOL(ftruncate);
EXPORT_SYMBOL(lseek);
EXPORT_SYMBOL(rewind);
#endif /* CONFIG_POSIX_FD_MGMT */

/* FILE_ATTRIBUTES */
#ifdef CONFIG_POSIX_FILE_ATTRIBUTES
EXPORT_SYMBOL(chmod);
EXPORT_SYMBOL(chown);
EXPORT_SYMBOL(fchmod);
EXPORT_SYMBOL(fchown);
EXPORT_SYMBOL(umask);
#endif /* CONFIG_POSIX_FILE_ATTRIBUTES */

/* FILE_ATTRIBUTES_FD */
#ifdef CONFIG_POSIX_FILE_ATTRIBUTES_FD
EXPORT_SYMBOL(fchmodat);
EXPORT_SYMBOL(fchownat);
#endif /* CONFIG_POSIX_FILE_ATTRIBUTES_FD */

/* FILE_LOCKING */
#ifdef CONFIG_POSIX_FILE_LOCKING
EXPORT_SYMBOL(flockfile);
EXPORT_SYMBOL(ftrylockfile);
EXPORT_SYMBOL(funlockfile);
EXPORT_SYMBOL(getc_unlocked);
EXPORT_SYMBOL(getchar_unlocked);
EXPORT_SYMBOL(putc_unlocked);
EXPORT_SYMBOL(putchar_unlocked);
#endif /* CONFIG_POSIX_FILE_LOCKING */

/* FILE_SYSTEM */
#ifdef CONFIG_POSIX_FILE_SYSTEM
EXPORT_SYMBOL(access);
EXPORT_SYMBOL(chdir);
EXPORT_SYMBOL(closedir);
EXPORT_SYMBOL(creat);
EXPORT_SYMBOL(fchdir);
EXPORT_SYMBOL(fpathconf);
EXPORT_SYMBOL(fstat);
EXPORT_SYMBOL(fstatvfs);
EXPORT_SYMBOL(getcwd);
EXPORT_SYMBOL(link);
EXPORT_SYMBOL(mkdir);
EXPORT_SYMBOL(mkstemp);
EXPORT_SYMBOL(opendir);
EXPORT_SYMBOL(pathconf);
EXPORT_SYMBOL(readdir);
EXPORT_SYMBOL(remove);
EXPORT_SYMBOL(rename);
EXPORT_SYMBOL(rewinddir);
EXPORT_SYMBOL(rmdir);
EXPORT_SYMBOL(stat);
EXPORT_SYMBOL(statvfs);
EXPORT_SYMBOL(tmpfile);
EXPORT_SYMBOL(tmpnam);
EXPORT_SYMBOL(truncate);
EXPORT_SYMBOL(unlink);
EXPORT_SYMBOL(utime);
#endif /* CONFIG_POSIX_FILE_SYSTEM */

/* FILE_SYSTEM_FD */
#ifdef CONFIG_POSIX_FILE_SYSTEM_FD
EXPORT_SYMBOL(faccessat);
EXPORT_SYMBOL(fdopendir);
EXPORT_SYMBOL(fstatat);
EXPORT_SYMBOL(linkat);
EXPORT_SYMBOL(mkdirat);
EXPORT_SYMBOL(openat);
EXPORT_SYMBOL(renameat);
EXPORT_SYMBOL(unlinkat);
EXPORT_SYMBOL(utimensat);
#endif /* CONFIG_POSIX_FILE_SYSTEM_FD */

/* FILE_SYSTEM_GLOB */
#ifdef CONFIG_POSIX_FILE_SYSTEM_GLOB
EXPORT_SYMBOL(glob);
EXPORT_SYMBOL(globfree);
#endif /* CONFIG_POSIX_FILE_SYSTEM_GLOB */

/* FILE_SYSTEM_R */
#ifdef CONFIG_POSIX_FILE_SYSTEM_R
EXPORT_SYMBOL(readdir_r);
#endif /* CONFIG_POSIX_FILE_SYSTEM_R */

/* JOB_CONTROL */
#ifdef CONFIG_POSIX_JOB_CONTROL
EXPORT_SYMBOL(setpgid);
EXPORT_SYMBOL(tcgetpgrp);
EXPORT_SYMBOL(tcgetsid);
EXPORT_SYMBOL(tcsetpgrp);
#endif /* CONFIG_POSIX_JOB_CONTROL */

/* MAPPED_FILES */
#ifdef CONFIG_POSIX_MAPPED_FILES
EXPORT_SYMBOL(mmap);
EXPORT_SYMBOL(msync);
EXPORT_SYMBOL(munmap);
#endif /* CONFIG_POSIX_MAPPED_FILES */

/* MULTI_PROCESS */
#ifdef CONFIG_POSIX_MULTI_PROCESS
EXPORT_SYMBOL(_Exit);
EXPORT_SYMBOL(_exit);
EXPORT_SYMBOL(atexit);
EXPORT_SYMBOL(execl);
EXPORT_SYMBOL(execle);
EXPORT_SYMBOL(execlp);
EXPORT_SYMBOL(execv);
EXPORT_SYMBOL(execve);
EXPORT_SYMBOL(execvp);
EXPORT_SYMBOL(exit);
EXPORT_SYMBOL(fork);
EXPORT_SYMBOL(getpgid);
EXPORT_SYMBOL(getpgrp);
EXPORT_SYMBOL(getpid);
EXPORT_SYMBOL(getppid);
EXPORT_SYMBOL(getsid);
EXPORT_SYMBOL(setsid);
EXPORT_SYMBOL(sleep);
EXPORT_SYMBOL(times);
EXPORT_SYMBOL(wait);
EXPORT_SYMBOL(waitid);
EXPORT_SYMBOL(waitpid);
#endif /* CONFIG_POSIX_MULTI_PROCESS */

/* NETWORKING: htonl(), htons(), ntohl(), and ntohs() are macros over
 * inline byte swaps in <arpa/inet.h>, so images carry no reference
 */
#ifdef CONFIG_POSIX_NETWORKING
EXPORT_SYMBOL(accept);
EXPORT_SYMBOL(bind);
EXPORT_SYMBOL(connect);
EXPORT_SYMBOL(endhostent);
EXPORT_SYMBOL(endnetent);
EXPORT_SYMBOL(endprotoent);
EXPORT_SYMBOL(endservent);
EXPORT_SYMBOL(freeaddrinfo);
EXPORT_SYMBOL(gai_strerror);
EXPORT_SYMBOL(getaddrinfo);
EXPORT_SYMBOL(gethostent);
EXPORT_SYMBOL(gethostname);
EXPORT_SYMBOL(getnameinfo);
EXPORT_SYMBOL(getnetbyaddr);
EXPORT_SYMBOL(getnetbyname);
EXPORT_SYMBOL(getnetent);
EXPORT_SYMBOL(getpeername);
EXPORT_SYMBOL(getprotobyname);
EXPORT_SYMBOL(getprotobynumber);
EXPORT_SYMBOL(getprotoent);
EXPORT_SYMBOL(getservbyname);
EXPORT_SYMBOL(getservbyport);
EXPORT_SYMBOL(getservent);
EXPORT_SYMBOL(getsockname);
EXPORT_SYMBOL(getsockopt);
EXPORT_SYMBOL(if_freenameindex);
EXPORT_SYMBOL(if_indextoname);
EXPORT_SYMBOL(if_nameindex);
EXPORT_SYMBOL(if_nametoindex);
EXPORT_SYMBOL(inet_addr);
EXPORT_SYMBOL(inet_ntoa);
EXPORT_SYMBOL(inet_ntop);
EXPORT_SYMBOL(inet_pton);
EXPORT_SYMBOL(listen);
EXPORT_SYMBOL(recv);
EXPORT_SYMBOL(recvfrom);
EXPORT_SYMBOL(recvmsg);
EXPORT_SYMBOL(send);
EXPORT_SYMBOL(sendmsg);
EXPORT_SYMBOL(sendto);
EXPORT_SYMBOL(sethostent);
EXPORT_SYMBOL(setnetent);
EXPORT_SYMBOL(setprotoent);
EXPORT_SYMBOL(setservent);
EXPORT_SYMBOL(setsockopt);
EXPORT_SYMBOL(shutdown);
EXPORT_SYMBOL(sockatmark);
EXPORT_SYMBOL(socket);
EXPORT_SYMBOL(socketpair);
#endif /* CONFIG_POSIX_NETWORKING */

/* NON_PORTABLE */
#ifdef CONFIG_POSIX_NON_PORTABLE
EXPORT_SYMBOL(pthread_getname_np);
EXPORT_SYMBOL(pthread_setname_np);
EXPORT_SYMBOL(pthread_timedjoin_np);
EXPORT_SYMBOL(pthread_tryjoin_np);
#endif /* CONFIG_POSIX_NON_PORTABLE */

/* PIPE */
#ifdef CONFIG_POSIX_PIPE
EXPORT_SYMBOL(pipe);
#endif /* CONFIG_POSIX_PIPE */

/* POSIX_THREADS_EXT */
#ifdef CONFIG_POSIX_THREADS_EXT
EXPORT_SYMBOL(pthread_attr_getguardsize);
EXPORT_SYMBOL(pthread_attr_setguardsize);
EXPORT_SYMBOL(pthread_mutexattr_gettype);
EXPORT_SYMBOL(pthread_mutexattr_settype);
#endif /* CONFIG_POSIX_THREADS_EXT */

/* REALTIME_SIGNALS */
#ifdef CONFIG_POSIX_REALTIME_SIGNALS
EXPORT_SYMBOL(sigqueue);
EXPORT_SYMBOL(sigtimedwait);
EXPORT_SYMBOL(sigwaitinfo);
#endif /* CONFIG_POSIX_REALTIME_SIGNALS */

/* REGEXP */
#ifdef CONFIG_POSIX_REGEXP
EXPORT_SYMBOL(regcomp);
EXPORT_SYMBOL(regerror);
EXPORT_SYMBOL(regexec);
EXPORT_SYMBOL(regfree);
#endif /* CONFIG_POSIX_REGEXP */

/* RW_LOCKS */
#ifdef CONFIG_POSIX_RW_LOCKS
EXPORT_SYMBOL(pthread_rwlock_destroy);
EXPORT_SYMBOL(pthread_rwlock_init);
EXPORT_SYMBOL(pthread_rwlock_rdlock);
EXPORT_SYMBOL(pthread_rwlock_tryrdlock);
EXPORT_SYMBOL(pthread_rwlock_trywrlock);
EXPORT_SYMBOL(pthread_rwlock_unlock);
EXPORT_SYMBOL(pthread_rwlock_wrlock);
EXPORT_SYMBOL(pthread_rwlockattr_destroy);
EXPORT_SYMBOL(pthread_rwlockattr_getpshared);
EXPORT_SYMBOL(pthread_rwlockattr_init);
EXPORT_SYMBOL(pthread_rwlockattr_setpshared);
#endif /* CONFIG_POSIX_RW_LOCKS */

/* SEMAPHORES */
#ifdef CONFIG_POSIX_SEMAPHORES
EXPORT_SYMBOL(sem_close);
EXPORT_SYMBOL(sem_destroy);
EXPORT_SYMBOL(sem_getvalue);
EXPORT_SYMBOL(sem_init);
EXPORT_SYMBOL(sem_open);
EXPORT_SYMBOL(sem_post);
EXPORT_SYMBOL(sem_trywait);
EXPORT_SYMBOL(sem_unlink);
EXPORT_SYMBOL(sem_wait);
#endif /* CONFIG_POSIX_SEMAPHORES */

/* SIGNAL_JUMP */
#ifdef CONFIG_POSIX_SIGNAL_JUMP
EXPORT_SYMBOL(siglongjmp);
#endif /* CONFIG_POSIX_SIGNAL_JUMP */

/* SIGNALS */
#ifdef CONFIG_POSIX_SIGNALS
EXPORT_SYMBOL(abort);
EXPORT_SYMBOL(alarm);
EXPORT_SYMBOL(kill);
EXPORT_SYMBOL(pause);
EXPORT_SYMBOL(raise);
EXPORT_SYMBOL(sigaction);
EXPORT_SYMBOL(sigaddset);
EXPORT_SYMBOL(sigdelset);
EXPORT_SYMBOL(sigemptyset);
EXPORT_SYMBOL(sigfillset);
EXPORT_SYMBOL(sigismember);
EXPORT_SYMBOL(signal);
EXPORT_SYMBOL(sigpending);
EXPORT_SYMBOL(sigprocmask);
EXPORT_SYMBOL(sigsuspend);
EXPORT_SYMBOL(sigwait);
#endif /* CONFIG_POSIX_SIGNALS */

/* SIGNALS_EXT */
#ifdef CONFIG_POSIX_SIGNALS_EXT
EXPORT_SYMBOL(strsignal);
#endif /* CONFIG_POSIX_SIGNALS_EXT */

/* SINGLE_PROCESS */
#ifdef CONFIG_POSIX_SINGLE_PROCESS
EXPORT_SYMBOL(confstr);
EXPORT_SYMBOL(getenv);
EXPORT_SYMBOL(setenv);
EXPORT_SYMBOL(sysconf);
EXPORT_SYMBOL(uname);
EXPORT_SYMBOL(unsetenv);
#endif /* CONFIG_POSIX_SINGLE_PROCESS */

/* SPIN_LOCKS */
#ifdef CONFIG_POSIX_SPIN_LOCKS
EXPORT_SYMBOL(pthread_spin_destroy);
EXPORT_SYMBOL(pthread_spin_init);
EXPORT_SYMBOL(pthread_spin_lock);
EXPORT_SYMBOL(pthread_spin_trylock);
EXPORT_SYMBOL(pthread_spin_unlock);
#endif /* CONFIG_POSIX_SPIN_LOCKS */

/* SYMBOLIC_LINKS */
#ifdef CONFIG_POSIX_SYMBOLIC_LINKS
EXPORT_SYMBOL(lchown);
EXPORT_SYMBOL(lstat);
EXPORT_SYMBOL(readlink);
EXPORT_SYMBOL(symlink);
#endif /* CONFIG_POSIX_SYMBOLIC_LINKS */

/* SYMBOLIC_LINKS_FD */
#ifdef CONFIG_POSIX_SYMBOLIC_LINKS_FD
EXPORT_SYMBOL(readlinkat);
EXPORT_SYMBOL(symlinkat);
#endif /* CONFIG_POSIX_SYMBOLIC_LINKS_FD */

/* SYSTEM_DATABASE */
#ifdef CONFIG_POSIX_SYSTEM_DATABASE
EXPORT_SYMBOL(getgrgid);
EXPORT_SYMBOL(getgrnam);
EXPORT_SYMBOL(getpwnam);
EXPORT_SYMBOL(getpwuid);
#endif /* CONFIG_POSIX_SYSTEM_DATABASE */

/* SYSTEM_DATABASE_R */
#ifdef CONFIG_POSIX_SYSTEM_DATABASE_R
EXPORT_SYMBOL(getgrgid_r);
EXPORT_SYMBOL(getgrnam_r);
EXPORT_SYMBOL(getpwnam_r);
EXPORT_SYMBOL(getpwuid_r);
#endif /* CONFIG_POSIX_SYSTEM_DATABASE_R */

/* THREADS_BASE */
#ifdef CONFIG_POSIX_THREADS
EXPORT_SYMBOL(pthread_atfork);
EXPORT_SYMBOL(pthread_attr_destroy);
EXPORT_SYMBOL(pthread_attr_getdetachstate);
EXPORT_SYMBOL(pthread_attr_getschedparam);
EXPORT_SYMBOL(pthread_attr_init);
EXPORT_SYMBOL(pthread_attr_setdetachstate);
EXPORT_SYMBOL(pthread_attr_setschedparam);
EXPORT_SYMBOL(pthread_cancel);
EXPORT_SYMBOL(pthread_cond_broadcast);
EXPORT_SYMBOL(pthread_cond_destroy);
EXPORT_SYMBOL(pthread_cond_init);
EXPORT_SYMBOL(pthread_cond_signal);
EXPORT_SYMBOL(pthread_cond_timedwait);
EXPORT_SYMBOL(pthread_cond_wait);
EXPORT_SYMBOL(pthread_condattr_destroy);
EXPORT_SYMBOL(pthread_condattr_init);
EXPORT_SYMBOL(pthread_create);
EXPORT_SYMBOL(pthread_detach);
EXPORT_SYMBOL(pthread_equal);
EXPORT_SYMBOL(pthread_exit);
EXPORT_SYMBOL(pthread_getspecific);
EXPORT_SYMBOL(pthread_join);
EXPORT_SYMBOL(pthread_key_create);
EXPORT_SYMBOL(pthread_key_delete);
EXPORT_SYMBOL(pthread_kill);
EXPORT_SYMBOL(pthread_mutex_destroy);
EXPORT_SYMBOL(pthread_mutex_init);
EXPORT_SYMBOL(pthread_mutex_lock);
EXPORT_SYMBOL(pthread_mutex_timedlock);
EXPORT_SYMBOL(pthread_mutex_trylock);
EXPORT_SYMBOL(pthread_mutex_unlock);
EXPORT_SYMBOL(pthread_mutexattr_destroy);
EXPORT_SYMBOL(pthread_mutexattr_init);
EXPORT_SYMBOL(pthread_once);
EXPORT_SYMBOL(pthread_self);
EXPORT_SYMBOL(pthread_setcancelstate);
EXPORT_SYMBOL(pthread_setcanceltype);
EXPORT_SYMBOL(pthread_setspecific);
EXPORT_SYMBOL(pthread_sigmask);
EXPORT_SYMBOL(pthread_testcancel);
EXPORT_SYMBOL(sched_yield);
#endif /* CONFIG_POSIX_THREADS */

/* TIMERS, TIMEOUTS */
#ifdef CONFIG_POSIX_TIMERS
EXPORT_SYMBOL(clock_getres);
EXPORT_SYMBOL(clock_gettime);
EXPORT_SYMBOL(clock_settime);
EXPORT_SYMBOL(nanosleep);
EXPORT_SYMBOL(timer_create);
EXPORT_SYMBOL(timer_delete);
EXPORT_SYMBOL(timer_getoverrun);
EXPORT_SYMBOL(timer_gettime);
EXPORT_SYMBOL(timer_settime);
EXPORT_SYMBOL(pthread_rwlock_timedrdlock);
EXPORT_SYMBOL(pthread_rwlock_timedwrlock);
EXPORT_SYMBOL(sem_timedwait);
#endif /* CONFIG_POSIX_TIMERS */

/* USER_GROUPS */
#ifdef CONFIG_POSIX_USER_GROUPS
EXPORT_SYMBOL(getegid);
EXPORT_SYMBOL(geteuid);
EXPORT_SYMBOL(getgid);
EXPORT_SYMBOL(getgroups);
EXPORT_SYMBOL(getlogin);
EXPORT_SYMBOL(getuid);
EXPORT_SYMBOL(setegid);
EXPORT_SYMBOL(seteuid);
EXPORT_SYMBOL(setgid);
EXPORT_SYMBOL(setuid);
#endif /* CONFIG_POSIX_USER_GROUPS */

/* USER_GROUPS_R */
#ifdef CONFIG_POSIX_USER_GROUPS_R
EXPORT_SYMBOL(getlogin_r);
#endif /* CONFIG_POSIX_USER_GROUPS_R */

/* XSI_SINGLE_PROCESS */
#ifdef CONFIG_XSI_SINGLE_PROCESS
EXPORT_SYMBOL(gethostid);
EXPORT_SYMBOL(gettimeofday);
EXPORT_SYMBOL(putenv);
#endif /* CONFIG_XSI_SINGLE_PROCESS */

/* XSI_SYSTEM_LOGGING */
#ifdef CONFIG_XSI_SYSTEM_LOGGING
EXPORT_SYMBOL(closelog);
EXPORT_SYMBOL(openlog);
EXPORT_SYMBOL(setlogmask);
EXPORT_SYMBOL(syslog);
#endif /* CONFIG_XSI_SYSTEM_LOGGING */

/* XSI_THREADS_EXT */
#ifdef CONFIG_XSI_THREADS_EXT
EXPORT_SYMBOL(pthread_getconcurrency);
EXPORT_SYMBOL(pthread_setconcurrency);
#endif /* CONFIG_XSI_THREADS_EXT */

/* ASYNCHRONOUS_IO */
#ifdef CONFIG_SYS_AIO
EXPORT_SYMBOL(aio_cancel);
EXPORT_SYMBOL(aio_error);
EXPORT_SYMBOL(aio_fsync);
EXPORT_SYMBOL(aio_read);
EXPORT_SYMBOL(aio_return);
EXPORT_SYMBOL(aio_suspend);
EXPORT_SYMBOL(aio_write);
EXPORT_SYMBOL(lio_listio);
#endif /* CONFIG_SYS_AIO */

/* CPUTIME */
#ifdef CONFIG_POSIX_CPUTIME
EXPORT_SYMBOL(clock_getcpuclockid);
#endif /* CONFIG_POSIX_CPUTIME */

/* FSYNC */
#ifdef CONFIG_POSIX_FSYNC
EXPORT_SYMBOL(fsync);
#endif /* CONFIG_POSIX_FSYNC */

/* MEMLOCK */
#ifdef CONFIG_POSIX_MEMLOCK
EXPORT_SYMBOL(mlockall);
EXPORT_SYMBOL(munlockall);
#endif /* CONFIG_POSIX_MEMLOCK */

/* MEMLOCK_RANGE */
#ifdef CONFIG_POSIX_MEMLOCK_RANGE
EXPORT_SYMBOL(mlock);
EXPORT_SYMBOL(munlock);
#endif /* CONFIG_POSIX_MEMLOCK_RANGE */

/* MESSAGE_PASSING */
#ifdef CONFIG_POSIX_MESSAGE_PASSING
EXPORT_SYMBOL(mq_close);
EXPORT_SYMBOL(mq_timedreceive);
EXPORT_SYMBOL(mq_timedsend);
EXPORT_SYMBOL(mq_getattr);
EXPORT_SYMBOL(mq_notify);
EXPORT_SYMBOL(mq_open);
EXPORT_SYMBOL(mq_receive);
EXPORT_SYMBOL(mq_send);
EXPORT_SYMBOL(mq_setattr);
EXPORT_SYMBOL(mq_unlink);
#endif /* CONFIG_POSIX_MESSAGE_PASSING */

/* PRIORITY_SCHEDULING */
#ifdef CONFIG_POSIX_PRIORITY_SCHEDULING
EXPORT_SYMBOL(sched_get_priority_max);
EXPORT_SYMBOL(sched_get_priority_min);
EXPORT_SYMBOL(sched_getparam);
EXPORT_SYMBOL(sched_getscheduler);
EXPORT_SYMBOL(sched_rr_get_interval);
EXPORT_SYMBOL(sched_setparam);
EXPORT_SYMBOL(sched_setscheduler);
#endif /* CONFIG_POSIX_PRIORITY_SCHEDULING */

/* SHARED_MEMORY_OBJECTS */
#ifdef CONFIG_POSIX_SHARED_MEMORY_OBJECTS
EXPORT_SYMBOL(shm_open);
EXPORT_SYMBOL(shm_unlink);
#endif /* CONFIG_POSIX_SHARED_MEMORY_OBJECTS */

/* SPAWN */
#ifdef CONFIG_POSIX_SPAWN
EXPORT_SYMBOL(posix_spawn);
EXPORT_SYMBOL(posix_spawn_file_actions_addclose);
EXPORT_SYMBOL(posix_spawn_file_actions_adddup2);
EXPORT_SYMBOL(posix_spawn_file_actions_addopen);
EXPORT_SYMBOL(posix_spawn_file_actions_destroy);
EXPORT_SYMBOL(posix_spawn_file_actions_init);
EXPORT_SYMBOL(posix_spawnattr_destroy);
EXPORT_SYMBOL(posix_spawnattr_getflags);
EXPORT_SYMBOL(posix_spawnattr_getpgroup);
EXPORT_SYMBOL(posix_spawnattr_getschedparam);
EXPORT_SYMBOL(posix_spawnattr_getschedpolicy);
EXPORT_SYMBOL(posix_spawnattr_getsigdefault);
EXPORT_SYMBOL(posix_spawnattr_getsigmask);
EXPORT_SYMBOL(posix_spawnattr_init);
EXPORT_SYMBOL(posix_spawnattr_setflags);
EXPORT_SYMBOL(posix_spawnattr_setpgroup);
EXPORT_SYMBOL(posix_spawnattr_setschedparam);
EXPORT_SYMBOL(posix_spawnattr_setschedpolicy);
EXPORT_SYMBOL(posix_spawnattr_setsigdefault);
EXPORT_SYMBOL(posix_spawnattr_setsigmask);
EXPORT_SYMBOL(posix_spawnp);
#endif /* CONFIG_POSIX_SPAWN */

/* SYNCHRONIZED_IO */
#ifdef CONFIG_POSIX_SYNCHRONIZED_IO
EXPORT_SYMBOL(fdatasync);
#endif /* CONFIG_POSIX_SYNCHRONIZED_IO */

/* THREAD_ATTR_STACKADDR */
#ifdef CONFIG_POSIX_THREAD_ATTR_STACKADDR
EXPORT_SYMBOL(pthread_attr_getstack);
EXPORT_SYMBOL(pthread_attr_setstack);
#endif /* CONFIG_POSIX_THREAD_ATTR_STACKADDR */

/* THREAD_ATTR_STACKSIZE */
#ifdef CONFIG_POSIX_THREAD_ATTR_STACKSIZE
EXPORT_SYMBOL(pthread_attr_getstacksize);
EXPORT_SYMBOL(pthread_attr_setstacksize);
#endif /* CONFIG_POSIX_THREAD_ATTR_STACKSIZE */

/* THREAD_CPUTIME */
#ifdef CONFIG_POSIX_THREAD_CPUTIME
EXPORT_SYMBOL(pthread_getcpuclockid);
#endif /* CONFIG_POSIX_THREAD_CPUTIME */

/* THREAD_PRIO_INHERIT */
#ifdef CONFIG_POSIX_THREAD_PRIO_INHERIT
EXPORT_SYMBOL(pthread_mutexattr_getprotocol);
EXPORT_SYMBOL(pthread_mutexattr_setprotocol);
#endif /* CONFIG_POSIX_THREAD_PRIO_INHERIT */

/* THREAD_PRIO_PROTECT */
#ifdef CONFIG_POSIX_THREAD_PRIO_PROTECT
EXPORT_SYMBOL(pthread_mutex_getprioceiling);
EXPORT_SYMBOL(pthread_mutex_setprioceiling);
EXPORT_SYMBOL(pthread_mutexattr_getprioceiling);
EXPORT_SYMBOL(pthread_mutexattr_setprioceiling);
#endif /* CONFIG_POSIX_THREAD_PRIO_PROTECT */

/* THREAD_PRIORITY_SCHEDULING */
#ifdef CONFIG_POSIX_THREAD_PRIORITY_SCHEDULING
EXPORT_SYMBOL(pthread_attr_getinheritsched);
EXPORT_SYMBOL(pthread_attr_getschedpolicy);
EXPORT_SYMBOL(pthread_attr_getscope);
EXPORT_SYMBOL(pthread_attr_setinheritsched);
EXPORT_SYMBOL(pthread_attr_setschedpolicy);
EXPORT_SYMBOL(pthread_attr_setscope);
EXPORT_SYMBOL(pthread_getschedparam);
EXPORT_SYMBOL(pthread_setschedparam);
EXPORT_SYMBOL(pthread_setschedprio);
#endif /* CONFIG_POSIX_THREAD_PRIORITY_SCHEDULING */

/* XSI_STREAMS */
#ifdef CONFIG_XSI_STREAMS
EXPORT_SYMBOL(fattach);
EXPORT_SYMBOL(fdetach);
EXPORT_SYMBOL(getmsg);
EXPORT_SYMBOL(getpmsg);
EXPORT_SYMBOL(ioctl);
EXPORT_SYMBOL(isastream);
EXPORT_SYMBOL(putmsg);
#endif /* CONFIG_XSI_STREAMS */


/* the ISO C surface (POSIX_C_LANG_SUPPORT, POSIX_C_LANG_MATH): provided
 * by the C library for every configuration
 */
EXPORT_SYMBOL(isalnum);
EXPORT_SYMBOL(isalpha);
EXPORT_SYMBOL(isblank);
EXPORT_SYMBOL(iscntrl);
EXPORT_SYMBOL(isdigit);
EXPORT_SYMBOL(isgraph);
EXPORT_SYMBOL(islower);
EXPORT_SYMBOL(isprint);
EXPORT_SYMBOL(ispunct);
EXPORT_SYMBOL(isspace);
EXPORT_SYMBOL(isupper);
EXPORT_SYMBOL(isxdigit);
EXPORT_SYMBOL(tolower);
EXPORT_SYMBOL(toupper);
EXPORT_SYMBOL(memchr);
EXPORT_SYMBOL(memcmp);
EXPORT_SYMBOL(memcpy);
EXPORT_SYMBOL(memmove);
EXPORT_SYMBOL(memset);
EXPORT_SYMBOL(strcat);
EXPORT_SYMBOL(strchr);
EXPORT_SYMBOL(strcmp);
EXPORT_SYMBOL(strcoll);
EXPORT_SYMBOL(strcpy);
EXPORT_SYMBOL(strcspn);
EXPORT_SYMBOL(strerror);
EXPORT_SYMBOL(strlen);
EXPORT_SYMBOL(strncat);
EXPORT_SYMBOL(strncmp);
EXPORT_SYMBOL(strncpy);
EXPORT_SYMBOL(strpbrk);
EXPORT_SYMBOL(strrchr);
EXPORT_SYMBOL(strspn);
EXPORT_SYMBOL(strstr);
EXPORT_SYMBOL(strtok);
EXPORT_SYMBOL(strxfrm);
EXPORT_SYMBOL(abs);
EXPORT_SYMBOL(atof);
EXPORT_SYMBOL(atoi);
EXPORT_SYMBOL(atol);
EXPORT_SYMBOL(atoll);
EXPORT_SYMBOL(bsearch);
EXPORT_SYMBOL(calloc);
EXPORT_SYMBOL(free);
EXPORT_SYMBOL(labs);
EXPORT_SYMBOL(llabs);
EXPORT_SYMBOL(malloc);
EXPORT_SYMBOL(qsort);
EXPORT_SYMBOL(rand);
EXPORT_SYMBOL(realloc);
EXPORT_SYMBOL(srand);
EXPORT_SYMBOL(strtod);
EXPORT_SYMBOL(strtof);
EXPORT_SYMBOL(strtol);
EXPORT_SYMBOL(strtold);
EXPORT_SYMBOL(strtoll);
EXPORT_SYMBOL(strtoul);
EXPORT_SYMBOL(strtoull);
EXPORT_SYMBOL(snprintf);
EXPORT_SYMBOL(sprintf);
EXPORT_SYMBOL(sscanf);
EXPORT_SYMBOL(vsnprintf);
EXPORT_SYMBOL(vsprintf);
EXPORT_SYMBOL(asctime);
EXPORT_SYMBOL(clock);
EXPORT_SYMBOL(ctime);
EXPORT_SYMBOL(difftime);
EXPORT_SYMBOL(gmtime);
EXPORT_SYMBOL(localtime);
EXPORT_SYMBOL(mktime);
EXPORT_SYMBOL(strftime);
EXPORT_SYMBOL(time);
EXPORT_SYMBOL(localeconv);
EXPORT_SYMBOL(setlocale);
EXPORT_SYMBOL(acos);
EXPORT_SYMBOL(asin);
EXPORT_SYMBOL(atan);
EXPORT_SYMBOL(atan2);
EXPORT_SYMBOL(ceil);
EXPORT_SYMBOL(cos);
EXPORT_SYMBOL(cosh);
EXPORT_SYMBOL(exp);
EXPORT_SYMBOL(fabs);
EXPORT_SYMBOL(floor);
EXPORT_SYMBOL(fmod);
EXPORT_SYMBOL(frexp);
EXPORT_SYMBOL(ldexp);
EXPORT_SYMBOL(log);
EXPORT_SYMBOL(log10);
EXPORT_SYMBOL(modf);
EXPORT_SYMBOL(pow);
EXPORT_SYMBOL(sin);
EXPORT_SYMBOL(sinh);
EXPORT_SYMBOL(sqrt);
EXPORT_SYMBOL(tan);
EXPORT_SYMBOL(tanh);

/* object-like identifiers of the groups above */
#ifdef CONFIG_POSIX_SINGLE_PROCESS
extern char **environ;
EXPORT_SYMBOL(environ);
#endif /* CONFIG_POSIX_SINGLE_PROCESS */
#ifdef CONFIG_POSIX_DEVICE_IO
EXPORT_SYMBOL(stdin);
EXPORT_SYMBOL(stdout);
EXPORT_SYMBOL(stderr);
#endif /* CONFIG_POSIX_DEVICE_IO */
#ifdef CONFIG_POSIX_C_LIB_EXT
EXPORT_SYMBOL(optarg);
EXPORT_SYMBOL(opterr);
EXPORT_SYMBOL(optind);
EXPORT_SYMBOL(optopt);
#ifdef CONFIG_ZEPHYR_GETOPT
EXPORT_SYMBOL(zephyr_getopt);
#endif /* CONFIG_ZEPHYR_GETOPT */
#endif /* CONFIG_POSIX_C_LIB_EXT */

