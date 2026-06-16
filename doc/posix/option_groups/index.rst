POSIX Option and Option Group Details
#####################################

.. _posix_option_groups:

POSIX Option Groups
===================

.. _posix_option_group_barriers:

POSIX_BARRIERS
++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_BARRIERS`.

.. csv-table:: POSIX_BARRIERS
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_barrier_destroy`,yes
    :c:func:`pthread_barrier_init`,yes
    :c:func:`pthread_barrier_wait`,yes
    :c:func:`pthread_barrierattr_destroy`,yes
    :c:func:`pthread_barrierattr_getpshared`,yes
    :c:func:`pthread_barrierattr_init`,yes
    :c:func:`pthread_barrierattr_setpshared`,yes

.. doxygengroup:: posix_option_group_barriers
   :project: posix

.. _posix_option_group_c_lang_jump:

POSIX_C_LANG_JUMP
+++++++++++++++++

The ``POSIX_C_LANG_JUMP`` Option Group is included in the ISO C standard.

.. note::
   When using Newlib, Picolibc, or other C libraries conforming to the ISO C Standard, the
   ``POSIX_C_LANG_JUMP`` Option Group is considered supported.

.. csv-table:: POSIX_C_LANG_JUMP
   :header: API, Supported
   :widths: 50,10

    :c:func:`setjmp`, yes
    :c:func:`longjmp`, yes

.. _posix_option_group_c_lang_math:

POSIX_C_LANG_MATH
+++++++++++++++++

The ``POSIX_C_LANG_MATH`` Option Group is included in the ISO C standard.

.. note::
   When using Newlib, Picolibc, or other C libraries conforming to the ISO C Standard, the
   ``POSIX_C_LANG_MATH`` Option Group is considered supported.

Please refer to `Subprofiling Considerations`_ for details on the ``POSIX_C_LANG_MATH`` Option
Group.

.. _posix_option_group_c_lang_support:

POSIX_C_LANG_SUPPORT
++++++++++++++++++++

The POSIX_C_LANG_SUPPORT option group contains the general ISO C Library.

.. note::
   When using Newlib, Picolibc, or other C libraries conforming to the ISO C Standard, the entire
   ``POSIX_C_LANG_SUPPORT`` Option Group is considered supported.

Please refer to `Subprofiling Considerations`_ for details on the ``POSIX_C_LANG_SUPPORT`` Option
Group.

For more information on developing Zephyr applications in the C programming language, please refer
to :ref:`details<language_support>`.

.. _posix_option_group_c_lang_support_r:

POSIX_C_LANG_SUPPORT_R
++++++++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_C_LANG_SUPPORT_R`.

.. csv-table:: POSIX_C_LANG_SUPPORT_R
   :header: API, Supported
   :widths: 50,10

    :c:func:`asctime_r`,yes
    :c:func:`ctime_r`,yes
    :c:func:`gmtime_r`,yes
    :c:func:`localtime_r`,yes
    :c:func:`qsort_r`,yes
    :c:func:`rand_r`,yes
    :c:func:`strerror_r`,yes
    :c:func:`strtok_r`,yes

.. doxygengroup:: posix_option_group_c_lang_support_r
   :project: posix

.. _posix_option_group_c_lib_ext:

POSIX_C_LIB_EXT
+++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_C_LIB_EXT`.

.. csv-table:: POSIX_C_LIB_EXT
   :header: API, Supported
   :widths: 50,10

    :c:func:`fnmatch`, yes
    :c:func:`getopt`, yes
    :c:func:`getsubopt`,
    optarg, yes
    opterr, yes
    optind, yes
    optopt, yes
    :c:func:`stpcpy`,
    :c:func:`stpncpy`,
    :c:func:`strcasecmp`,
    :c:func:`strdup`,
    :c:func:`strfmon`,
    :c:func:`strncasecmp`, yes
    :c:func:`strndup`,
    :c:func:`strnlen`, yes

.. doxygengroup:: posix_option_group_c_lib_ext
   :project: posix

.. _posix_option_group_clock_selection:

POSIX_CLOCK_SELECTION
+++++++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_CLOCK_SELECTION`.

.. csv-table:: POSIX_CLOCK_SELECTION
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_condattr_getclock`,yes
    :c:func:`pthread_condattr_setclock`,yes
    :c:func:`clock_nanosleep`,yes

.. doxygengroup:: posix_option_group_clock_selection
   :project: posix

.. _posix_option_group_device_io:

POSIX_DEVICE_IO
+++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_DEVICE_IO`.

.. note::
   When using Newlib, Picolibc, or other C libraries conforming to the ISO C Standard, the
   C89 components of the ``POSIX_DEVICE_IO`` Option Group are considered supported.

.. csv-table:: POSIX_DEVICE_IO
   :header: API, Supported
   :widths: 50,10

    :c:func:`FD_CLR`,yes
    :c:func:`FD_ISSET`,yes
    :c:func:`FD_SET`,yes
    :c:func:`FD_ZERO`,yes
    :c:func:`clearerr`,yes
    :c:func:`close`,yes
    :c:func:`fclose`,yes
    :c:func:`fdopen`,yes
    :c:func:`feof`,yes
    :c:func:`ferror`,yes
    :c:func:`fflush`,yes
    :c:func:`fgetc`,yes
    :c:func:`fgets`,yes
    :c:func:`fileno`,yes
    :c:func:`fopen`,yes
    :c:func:`fprintf`,yes
    :c:func:`fputc`,yes
    :c:func:`fputs`,yes
    :c:func:`fread`,yes
    :c:func:`freopen`,yes
    :c:func:`fscanf`,yes
    :c:func:`fwrite`,yes
    :c:func:`getc`,yes
    :c:func:`getchar`,yes
    :c:func:`gets`,yes
    :c:func:`open`,yes
    :c:func:`perror`,yes
    :c:func:`poll`,yes
    :c:func:`printf`,yes
    :c:func:`pread`,yes
    :c:func:`pselect`,yes
    :c:func:`putc`,yes
    :c:func:`putchar`,yes
    :c:func:`puts`,yes
    :c:func:`pwrite`,yes
    :c:func:`read`,yes
    :c:func:`scanf`,yes
    :c:func:`select`,yes
    :c:func:`setbuf`,yes
    :c:func:`setvbuf`,yes
    stderr,yes
    stdin,yes
    stdout,yes
    :c:func:`ungetc`,yes
    :c:func:`vfprintf`,yes
    :c:func:`vfscanf`,yes
    :c:func:`vprintf`,yes
    :c:func:`vscanf`,yes
    :c:func:`write`,yes

.. doxygengroup:: posix_option_group_device_io
   :project: posix

.. _posix_option_group_fd_mgmt:

POSIX_FD_MGMT
+++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_FD_MGMT`.

.. csv-table:: POSIX_FD_MGMT
   :header: API, Supported
   :widths: 50,10

    :c:func:`dup`,
    :c:func:`dup2`,
    :c:func:`fcntl`,
    :c:func:`fgetpos`,
    :c:func:`fseek`,
    :c:func:`fseeko`,
    :c:func:`fsetpos`,
    :c:func:`ftell`,
    :c:func:`ftello`,
    :c:func:`ftruncate`,yes
    :c:func:`lseek`,
    :c:func:`rewind`,

.. doxygengroup:: posix_option_group_fd_mgmt
   :project: posix

.. _posix_option_group_file_locking:

POSIX_FILE_LOCKING
++++++++++++++++++

.. csv-table:: POSIX_FILE_LOCKING
   :header: API, Supported
   :widths: 50,10

    :c:func:`flockfile`,
    :c:func:`ftrylockfile`,
    :c:func:`funlockfile`,
    :c:func:`getc_unlocked`,
    :c:func:`getchar_unlocked`,
    :c:func:`putc_unlocked`,
    :c:func:`putchar_unlocked`,

.. doxygengroup:: posix_option_group_file_locking
   :project: posix

.. _posix_option_group_file_system:

POSIX_FILE_SYSTEM
+++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_FILE_SYSTEM`.

.. csv-table:: POSIX_FILE_SYSTEM
   :header: API, Supported
   :widths: 50,10

    :c:func:`access`,
    :c:func:`chdir`,
    :c:func:`closedir`, yes
    :c:func:`creat`,
    :c:func:`fchdir`,
    :c:func:`fpathconf`,
    :c:func:`fstat`, yes
    :c:func:`fstatvfs`,
    :c:func:`getcwd`,
    :c:func:`link`,
    :c:func:`mkdir`, yes
    :c:func:`mkstemp`,
    :c:func:`opendir`, yes
    :c:func:`pathconf`,
    :c:func:`readdir`, yes
    :c:func:`remove`, yes
    :c:func:`rename`, yes
    :c:func:`rewinddir`,
    :c:func:`rmdir`, yes
    :c:func:`stat`, yes
    :c:func:`statvfs`,
    :c:func:`tmpfile`,
    :c:func:`tmpnam`,
    :c:func:`truncate`,
    :c:func:`unlink`, yes
    :c:func:`utime`,

.. doxygengroup:: posix_option_group_file_system
   :project: posix

.. _posix_option_group_file_system_r:

POSIX_FILE_SYSTEM_R
+++++++++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_FILE_SYSTEM_R`.

.. csv-table:: POSIX_FILE_SYSTEM_R
   :header: API, Supported
   :widths: 50,10

    :c:func:`readdir_r`, yes

.. _posix_option_group_mapped_files:

POSIX_MAPPED_FILES
++++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_MAPPED_FILES`.

.. csv-table:: POSIX_MAPPED_FILES
   :header: API, Supported
   :widths: 50,10

    :c:func:`mmap`,yes
    :c:func:`msync`,yes
    :c:func:`munmap`,yes

.. doxygengroup:: posix_option_group_mapped_files
   :project: posix

.. _posix_option_group_memory_protection:

POSIX_MEMORY_PROTECTION
+++++++++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_MEMORY_PROTECTION`.

.. csv-table:: POSIX_MEMORY_PROTECTION
   :header: API, Supported
   :widths: 50,10

    :c:func:`mprotect`, yes :ref:`†<posix_undefined_behaviour>`

.. _posix_option_group_multi_process:

POSIX_MULTI_PROCESS
+++++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_MULTI_PROCESS`.

.. csv-table:: POSIX_MULTI_PROCESS
   :header: API, Supported
   :widths: 50,10

    :c:func:`_Exit`, yes
    :c:func:`_exit`, yes
    :c:func:`assert`, yes
    :c:func:`atexit`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`clock`,
    :c:func:`execl`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`execle`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`execlp`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`execv`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`execve`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`execvp`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`exit`, yes
    :c:func:`fork`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`getpgrp`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`getpgid`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`getpid`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`getppid`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`getsid`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`setsid`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`sleep`,yes
    :c:func:`times`,
    :c:func:`wait`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`waitid`,:ref:`†<posix_undefined_behaviour>`
    :c:func:`waitpid`,:ref:`†<posix_undefined_behaviour>`

.. doxygengroup:: posix_option_group_multi_process
   :project: posix

.. _posix_option_group_networking:

POSIX_NETWORKING
++++++++++++++++

The function ``sockatmark()`` is not yet supported and is expected to fail setting ``errno``
to ``ENOSYS`` :ref:`†<posix_undefined_behaviour>`.

Enable this option group with :kconfig:option:`CONFIG_POSIX_NETWORKING`.

.. csv-table:: POSIX_NETWORKING
   :header: API, Supported
   :widths: 50,10

    :c:func:`accept`,yes
    :c:func:`bind`,yes
    :c:func:`connect`,yes
    :c:func:`endhostent`,yes
    :c:func:`endnetent`,yes
    :c:func:`endprotoent`,yes
    :c:func:`endservent`,yes
    :c:func:`freeaddrinfo`,yes
    :c:func:`gai_strerror`,yes
    :c:func:`getaddrinfo`,yes
    :c:func:`gethostent`,yes
    :c:func:`gethostname`,yes
    :c:func:`getnameinfo`,yes
    :c:func:`getnetbyaddr`,yes
    :c:func:`getnetbyname`,yes
    :c:func:`getnetent`,yes
    :c:func:`getpeername`,yes
    :c:func:`getprotobyname`,yes
    :c:func:`getprotobynumber`,yes
    :c:func:`getprotoent`,yes
    :c:func:`getservbyname`,yes
    :c:func:`getservbyport`,yes
    :c:func:`getservent`,yes
    :c:func:`getsockname`,yes
    :c:func:`getsockopt`,yes
    :c:func:`htonl`,yes
    :c:func:`htons`,yes
    :c:func:`if_freenameindex`,yes
    :c:func:`if_indextoname`,yes
    :c:func:`if_nameindex`,yes
    :c:func:`if_nametoindex`,yes
    :c:func:`inet_addr`,yes
    :c:func:`inet_ntoa`,yes
    :c:func:`inet_ntop`,yes
    :c:func:`inet_pton`,yes
    :c:func:`listen`,yes
    :c:func:`ntohl`,yes
    :c:func:`ntohs`,yes
    :c:func:`recv`,yes
    :c:func:`recvfrom`,yes
    :c:func:`recvmsg`,yes
    :c:func:`send`,yes
    :c:func:`sendmsg`,yes
    :c:func:`sendto`,yes
    :c:func:`sethostent`,yes
    :c:func:`setnetent`,yes
    :c:func:`setprotoent`,yes
    :c:func:`setservent`,yes
    :c:func:`setsockopt`,yes
    :c:func:`shutdown`,yes
    :c:func:`socket`,yes
    :c:func:`sockatmark`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`socketpair`,yes

.. _posix_option_group_pipe:

POSIX_PIPE
++++++++++

.. csv-table:: POSIX_PIPE
   :header: API, Supported
   :widths: 50,10

    :c:func:`pipe`,

.. doxygengroup:: posix_option_group_pipe
   :project: posix

.. _posix_option_group_realtime_signals:

POSIX_REALTIME_SIGNALS
++++++++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_REALTIME_SIGNALS`.

.. csv-table:: POSIX_REALTIME_SIGNALS
   :header: API, Supported
   :widths: 50,10

    :c:func:`sigqueue`,
    :c:func:`sigtimedwait`,
    :c:func:`sigwaitinfo`,

..
   this link is "deprecated" - mainly left here so that older links still work

.. doxygengroup:: posix_option_group_realtime_signals
   :project: posix

.. _posix_option_reader_writer_locks:

.. _posix_option_group_rw_locks:

POSIX_RW_LOCKS
++++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_RW_LOCKS`.

.. csv-table:: POSIX_RW_LOCKS
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_rwlock_destroy`,yes
    :c:func:`pthread_rwlock_init`,yes
    :c:func:`pthread_rwlock_rdlock`,yes
    :c:func:`pthread_rwlock_tryrdlock`,yes
    :c:func:`pthread_rwlock_trywrlock`,yes
    :c:func:`pthread_rwlock_unlock`,yes
    :c:func:`pthread_rwlock_wrlock`,yes
    :c:func:`pthread_rwlockattr_destroy`,yes
    :c:func:`pthread_rwlockattr_getpshared`,yes
    :c:func:`pthread_rwlockattr_init`,yes
    :c:func:`pthread_rwlockattr_setpshared`,yes

.. doxygengroup:: posix_option_group_rw_locks
   :project: posix

.. _posix_option_group_semaphores:

POSIX_SEMAPHORES
++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_SEMAPHORES`.

.. csv-table:: POSIX_SEMAPHORES
   :header: API, Supported
   :widths: 50,10

    :c:func:`sem_close`,yes
    :c:func:`sem_destroy`,yes
    :c:func:`sem_getvalue`,yes
    :c:func:`sem_init`,yes
    :c:func:`sem_open`,yes
    :c:func:`sem_post`,yes
    :c:func:`sem_trywait`,yes
    :c:func:`sem_unlink`,yes
    :c:func:`sem_wait`,yes

.. _posix_option_group_signal_jump:

POSIX_SIGNAL_JUMP
+++++++++++++++++

.. csv-table:: POSIX_SIGNAL_JUMP
   :header: API, Supported
   :widths: 50,10

    :c:func:`siglongjmp`,
    :c:func:`sigsetjmp`,

.. _posix_option_group_signals:

POSIX_SIGNALS
+++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_SIGNALS`.

.. note::
   As processes are not yet supported in Zephyr, the ISO C functions ``abort()``, ``signal()``,
   and ``raise()``, as well as the other POSIX functions listed below, may exhibit undefined
   behaviour. The POSIX functions ``kill()``, ``pause()``, ``sigaction()``, ``sigpending()``,
   ``sigsuspend()``, and ``sigwait()`` are implemented to ensure that conformant applications can
   link, but they are expected to fail, setting errno to ``ENOSYS``
   :ref:`†<posix_undefined_behaviour>`.

.. csv-table:: POSIX_SIGNALS
   :header: API, Supported
   :widths: 50,10

    :c:func:`abort`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`alarm`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`kill`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pause`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`raise`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sigaction`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sigaddset`,yes
    :c:func:`sigdelset`,yes
    :c:func:`sigemptyset`,yes
    :c:func:`sigfillset`,yes
    :c:func:`sigismember`,yes
    :c:func:`signal`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sigpending`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sigprocmask`,yes
    :c:func:`sigsuspend`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sigwait`,yes :ref:`†<posix_undefined_behaviour>`

.. doxygengroup:: posix_option_group_signals
   :project: posix

.. _posix_option_group_signals_ext:

POSIX_SIGNALS_EXT
+++++++++++++++++

Extended signal helpers from V4 subprofiles (``POSIX_SIGNALS_EXT``).

.. csv-table:: POSIX_SIGNALS_EXT
   :header: API, Supported
   :widths: 50,10

    :c:func:`psignal`,
    :c:func:`psiginfo`,
    :c:func:`strsignal`,yes

.. doxygengroup:: posix_option_group_signals_ext
   :project: posix

.. _posix_option_group_single_process:

POSIX_SINGLE_PROCESS
++++++++++++++++++++

The POSIX_SINGLE_PROCESS option group contains services for single
process applications.

Enable this option group with :kconfig:option:`CONFIG_POSIX_SINGLE_PROCESS`.

.. csv-table:: POSIX_SINGLE_PROCESS
   :header: API, Supported
   :widths: 50,10

    :c:func:`confstr`,yes
    environ,yes
    errno,yes
    :c:func:`getenv`,yes
    :c:func:`setenv`,yes
    :c:func:`sysconf`,yes
    :c:func:`uname`,yes
    :c:func:`unsetenv`,yes

.. doxygengroup:: posix_option_group_single_process
   :project: posix

.. _posix_option_group_system_database_r:

POSIX_SYSTEM_DATABASE_R
+++++++++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_SYSTEM_DATABASE_R`.

.. csv-table:: POSIX_SYSTEM_DATABASE_R
   :header: API, Supported
   :widths: 50,10

    :c:func:`getgrgid_r`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`getgrnam_r`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`getpwnam_r`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`getpwuid_r`,yes :ref:`†<posix_undefined_behaviour>`

.. doxygengroup:: posix_option_group_system_database_r
   :project: posix

.. _posix_option_group_spin_locks:

POSIX_SPIN_LOCKS
++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_SPIN_LOCKS`.

.. csv-table:: POSIX_SPIN_LOCKS
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_spin_destroy`,yes
    :c:func:`pthread_spin_init`,yes
    :c:func:`pthread_spin_lock`,yes
    :c:func:`pthread_spin_trylock`,yes
    :c:func:`pthread_spin_unlock`,yes

.. doxygengroup:: posix_option_group_spin_locks
   :project: posix

.. _posix_option_group_threads_base:

POSIX_THREADS_BASE
++++++++++++++++++

The basic assumption in this profile is that the system
consists of a single (implicit) process with multiple threads. Therefore, the
standard requires all basic thread services, except those related to
multiple processes.

Enable this option group with :kconfig:option:`CONFIG_POSIX_THREADS`.

.. csv-table:: POSIX_THREADS_BASE
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_atfork`,yes
    :c:func:`pthread_attr_destroy`,yes
    :c:func:`pthread_attr_getdetachstate`,yes
    :c:func:`pthread_attr_getschedparam`,yes
    :c:func:`pthread_attr_init`,yes
    :c:func:`pthread_attr_setdetachstate`,yes
    :c:func:`pthread_attr_setschedparam`,yes
    :c:func:`pthread_cancel`,yes
    :c:func:`pthread_cleanup_pop`,yes
    :c:func:`pthread_cleanup_push`,yes
    :c:func:`pthread_cond_broadcast`,yes
    :c:func:`pthread_cond_destroy`,yes
    :c:func:`pthread_cond_init`,yes
    :c:func:`pthread_cond_signal`,yes
    :c:func:`pthread_cond_timedwait`,yes
    :c:func:`pthread_cond_wait`,yes
    :c:func:`pthread_condattr_destroy`,yes
    :c:func:`pthread_condattr_init`,yes
    :c:func:`pthread_create`,yes
    :c:func:`pthread_detach`,yes
    :c:func:`pthread_equal`,yes
    :c:func:`pthread_exit`,yes
    :c:func:`pthread_getspecific`,yes
    :c:func:`pthread_join`,yes
    :c:func:`pthread_key_create`,yes
    :c:func:`pthread_key_delete`,yes
    :c:func:`pthread_kill`,
    :c:func:`pthread_mutex_destroy`,yes
    :c:func:`pthread_mutex_init`,yes
    :c:func:`pthread_mutex_lock`,yes
    :c:func:`pthread_mutex_timedlock`,yes
    :c:func:`pthread_mutex_trylock`,yes
    :c:func:`pthread_mutex_unlock`,yes
    :c:func:`pthread_mutexattr_destroy`,yes
    :c:func:`pthread_mutexattr_init`,yes
    :c:func:`pthread_once`,yes
    :c:func:`pthread_self`,yes
    :c:func:`pthread_setcancelstate`,yes
    :c:func:`pthread_setcanceltype`,yes
    :c:func:`pthread_setspecific`,yes
    :c:func:`pthread_sigmask`,yes
    :c:func:`pthread_testcancel`,yes

.. doxygengroup:: posix_option_group_threads_base
   :project: posix

.. _posix_option_group_posix_threads_ext:

POSIX_THREADS_EXT
+++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_THREADS_EXT`.

.. csv-table:: POSIX_THREADS_EXT
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_attr_getguardsize`,yes
    :c:func:`pthread_attr_setguardsize`,yes
    :c:func:`pthread_mutexattr_gettype`,yes
    :c:func:`pthread_mutexattr_settype`,yes

.. doxygengroup:: posix_option_group_posix_threads_ext
   :project: posix

.. _posix_option_group_non_portable:

POSIX_NON_PORTABLE
++++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_NON_PORTABLE`.

.. csv-table:: POSIX_NON_PORTABLE
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_getname_np`,yes
    :c:func:`pthread_setname_np`,yes
    :c:func:`pthread_timedjoin_np`,yes
    :c:func:`pthread_tryjoin_np`,yes

.. doxygengroup:: posix_option_group_non_portable
   :project: posix

.. _posix_option_group_timers:

POSIX_TIMERS
++++++++++++

Enable this option group with :kconfig:option:`CONFIG_POSIX_TIMERS`.

.. csv-table:: POSIX_TIMERS
   :header: API, Supported
   :widths: 50,10

    :c:func:`clock_getres`,yes
    :c:func:`clock_gettime`,yes
    :c:func:`clock_settime`,yes
    :c:func:`nanosleep`,yes
    :c:func:`timer_create`,yes
    :c:func:`timer_delete`,yes
    :c:func:`timer_gettime`,yes
    :c:func:`timer_getoverrun`,yes
    :c:func:`timer_settime`,yes

.. doxygengroup:: posix_option_group_timers
   :project: posix

.. _posix_option_group_xsi_advanced_realtime:

XSI_ADVANCED_REALTIME
+++++++++++++++++++++

The ``XSI_ADVANCED_REALTIME`` option group indicates that the
:ref:`_POSIX_CLOCK_SELECTION<posix_option_group_clock_selection>`,
:ref:`_POSIX_CPUTIME<posix_option_cputime>`, and
:ref:`_POSIX_MONOTONIC_CLOCK<posix_option_monotonic_clock>` options are enabled.

Enable this option group with :kconfig:option:`CONFIG_XSI_ADVANCED_REALTIME`.

An implementation that claims conformance to this option group shall also support the
:ref:`POSIX_TIMERS <posix_option_group_timers>` option group.


.. _posix_option_group_xsi_realtime:

XSI_REALTIME
++++++++++++

The ``XSI_REALTIME`` option group indicates that the :ref:`_POSIX_FSYNC<posix_option_fsync>`,
:ref:`_POSIX_MEMLOCK<posix_option_memlock>`,
:ref:`_POSIX_MEMLOCK_RANGE<posix_option_memlock_range>`,
:ref:`_POSIX_MESSAGE_PASSING<posix_option_message_passing>`,
:ref:`_POSIX_PRIORITY_SCHEDULING<posix_option_priority_scheduling>`,
:ref:`_POSIX_SHARED_MEMORY_OBJECTS<posix_option_shared_memory_objects>`, and
:ref:`_POSIX_SYNCHRONIZED_IO<posix_option_synchronized_io>` options are enabled.

Enable this option group with :kconfig:option:`CONFIG_XSI_REALTIME`.

When this option group is enabled, the ``_XOPEN_REALTIME`` feature test macro will be defined to a
value other than -1.

.. _posix_option_group_xsi_single_process:

XSI_SINGLE_PROCESS
++++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_XSI_SINGLE_PROCESS`.

.. csv-table:: XSI_SINGLE_PROCESS
   :header: API, Supported
   :widths: 50,10

    :c:func:`gethostid`,yes
    :c:func:`gettimeofday`,yes
    :c:func:`putenv`,yes

.. doxygengroup:: posix_option_group_xsi_single_process
   :project: posix

.. _posix_option_group_xsi_system_logging:

XSI_SYSTEM_LOGGING
++++++++++++++++++

Enable this option group with :kconfig:option:`CONFIG_XSI_SYSTEM_LOGGING`.

.. csv-table:: XSI_SYSTEM_LOGGING
   :header: API, Supported
   :widths: 50,10

    :c:func:`closelog`,yes
    :c:func:`openlog`,yes
    :c:func:`setlogmask`,yes
    :c:func:`syslog`,yes

.. doxygengroup:: posix_option_group_xsi_system_logging
   :project: posix

.. _posix_option_group_xsi_threads_ext:

XSI_THREADS_EXT
+++++++++++++++

The ``XSI_THREADS_EXT`` option group provides thread concurrency hints.

Enable this option group with :kconfig:option:`CONFIG_XSI_THREADS_EXT`.

Combined stack address and size control is provided by
:ref:`pthread_attr_getstack() <posix_option_thread_attr_stackaddr>` and
:ref:`pthread_attr_setstack() <posix_option_thread_attr_stackaddr>` when both
:ref:`_POSIX_THREAD_ATTR_STACKADDR <posix_option_thread_attr_stackaddr>` and
:ref:`_POSIX_THREAD_ATTR_STACKSIZE <posix_option_thread_attr_stacksize>` are
enabled.

.. csv-table:: XSI_THREADS_EXT
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_getconcurrency`,yes
    :c:func:`pthread_setconcurrency`,yes

.. doxygengroup:: posix_option_group_xsi_threads_ext
   :project: posix

.. _posix_option_group_xsi_realtime_threads:

XSI_REALTIME_THREADS
++++++++++++++++++++

The ``XSI_REALTIME_THREADS`` option group (``_XOPEN_REALTIME_THREADS``) enables
:ref:`_POSIX_THREAD_PRIORITY_SCHEDULING <posix_option_thread_priority_scheduling>`,
:ref:`_POSIX_THREAD_PRIO_INHERIT <posix_option_thread_prio_inherit>`, and
:ref:`_POSIX_THREAD_PRIO_PROTECT <posix_option_thread_prio_protect>`.

Enable this option group with :kconfig:option:`CONFIG_XSI_REALTIME_THREADS`.


.. _posix_option_group_xsi_advanced_realtime_threads:

XSI_ADVANCED_REALTIME_THREADS
+++++++++++++++++++++++++++++

The ``XSI_ADVANCED_REALTIME_THREADS`` option group enables
:ref:`_POSIX_THREAD_CPUTIME <posix_option_thread_cputime>` and requires
:ref:`XSI_REALTIME_THREADS <posix_option_group_xsi_realtime_threads>`.

Enable this option group with :kconfig:option:`CONFIG_XSI_ADVANCED_REALTIME_THREADS`.


.. _posix_options:

POSIX Options
=============

.. _posix_option_asynchronous_io:

_POSIX_ASYNCHRONOUS_IO
++++++++++++++++++++++

Functions part of the ``_POSIX_ASYNCHRONOUS_IO`` Option are not implemented in Zephyr but are
provided so that conformant applications can still link. These functions will fail, setting
``errno`` to ``ENOSYS``:ref:`†<posix_undefined_behaviour>`.

Enable this option with :kconfig:option:`CONFIG_POSIX_ASYNCHRONOUS_IO`.

.. csv-table:: _POSIX_ASYNCHRONOUS_IO
   :header: API, Supported
   :widths: 50,10

    :c:func:`aio_cancel`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`aio_error`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`aio_fsync`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`aio_read`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`aio_return`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`aio_suspend`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`aio_write`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`lio_listio`,yes :ref:`†<posix_undefined_behaviour>`

.. _posix_option_cputime:

_POSIX_CPUTIME
++++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_CPUTIME`, or enable the
:ref:`XSI_ADVANCED_REALTIME <posix_option_group_xsi_advanced_realtime>` option group with
:kconfig:option:`CONFIG_XSI_ADVANCED_REALTIME`.

.. csv-table:: _POSIX_CPUTIME
   :header: API, Supported
   :widths: 50,10

    CLOCK_PROCESS_CPUTIME_ID,yes
    :c:func:`clock_getcpuclockid`,yes

.. doxygengroup:: posix_option_cputime
   :project: posix

.. _posix_option_fsync:

_POSIX_FSYNC
++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_FSYNC`.

.. csv-table:: _POSIX_FSYNC
   :header: API, Supported
   :widths: 50,10

    :c:func:`fsync`,yes

.. doxygengroup:: posix_option_fsync
   :project: posix

.. _posix_option_ipv6:

_POSIX_IPV6
+++++++++++

Internet Protocol Version 6 is supported.

For more information, please refer to :ref:`Networking <networking>`.

Enable this option with :kconfig:option:`CONFIG_POSIX_IPV6`.

.. _posix_option_memlock:

_POSIX_MEMLOCK
++++++++++++++

Zephyr's :ref:`Demand Paging API <memory_management_api_demand_paging>` does not yet support
pinning or unpinning all virtual memory regions. The functions below are expected to fail and
set ``errno`` to ``ENOSYS`` :ref:`†<posix_undefined_behaviour>`.

Enable this option with :kconfig:option:`CONFIG_POSIX_MEMLOCK`.

.. csv-table:: _POSIX_MEMLOCK
   :header: API, Supported
   :widths: 50,10

    :c:func:`mlockall`, yes
    :c:func:`munlockall`, yes

.. doxygengroup:: posix_option_memlock
   :project: posix

.. _posix_option_memlock_range:

_POSIX_MEMLOCK_RANGE
++++++++++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_MEMLOCK_RANGE`.

.. csv-table:: _POSIX_MEMLOCK_RANGE
   :header: API, Supported
   :widths: 50,10

    :c:func:`mlock`, yes
    :c:func:`munlock`, yes

.. doxygengroup:: posix_option_memlock_range
   :project: posix

.. _posix_option_message_passing:

_POSIX_MESSAGE_PASSING
++++++++++++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_MESSAGE_PASSING`.

.. csv-table:: _POSIX_MESSAGE_PASSING
   :header: API, Supported
   :widths: 50,10

    :c:func:`mq_close`,yes
    :c:func:`mq_getattr`,yes
    :c:func:`mq_notify`,yes
    :c:func:`mq_open`,yes
    :c:func:`mq_receive`,yes
    :c:func:`mq_send`,yes
    :c:func:`mq_setattr`,yes
    :c:func:`mq_unlink`,yes

.. _posix_option_monotonic_clock:

_POSIX_MONOTONIC_CLOCK
++++++++++++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_MONOTONIC_CLOCK`, or enable the
:ref:`XSI_ADVANCED_REALTIME <posix_option_group_xsi_advanced_realtime>` option group with
:kconfig:option:`CONFIG_XSI_ADVANCED_REALTIME`.

.. csv-table:: _POSIX_MONOTONIC_CLOCK
   :header: API, Supported
   :widths: 50,10

    CLOCK_MONOTONIC,yes

.. doxygengroup:: posix_option_monotonic_clock
   :project: posix

.. _posix_option_priority_scheduling:

_POSIX_PRIORITY_SCHEDULING
++++++++++++++++++++++++++

As processes are not yet supported in Zephyr, the functions ``sched_rr_get_interval()``,
``sched_setparam()``, and ``sched_setscheduler()`` are expected to fail setting ``errno``
to ``ENOSYS``:ref:`†<posix_undefined_behaviour>`.

Enable this option with :kconfig:option:`CONFIG_POSIX_PRIORITY_SCHEDULING`.

.. csv-table:: _POSIX_PRIORITY_SCHEDULING
   :header: API, Supported
   :widths: 50,10

    :c:func:`sched_get_priority_max`,yes
    :c:func:`sched_get_priority_min`,yes
    :c:func:`sched_getparam`,yes
    :c:func:`sched_getscheduler`,yes
    :c:func:`sched_rr_get_interval`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sched_setparam`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sched_setscheduler`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sched_yield`,yes

.. doxygengroup:: posix_option_priority_scheduling
   :project: posix

.. _posix_option_raw_sockets:

_POSIX_RAW_SOCKETS
++++++++++++++++++

Raw sockets are supported.

For more information, please refer to :kconfig:option:`CONFIG_NET_SOCKETS_PACKET`.

Enable this option with :kconfig:option:`CONFIG_POSIX_RAW_SOCKETS`.

.. _posix_shared_memory_objects:

.. _posix_option_shared_memory_objects:

_POSIX_SHARED_MEMORY_OBJECTS
++++++++++++++++++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_SHARED_MEMORY_OBJECTS`.

.. csv-table:: _POSIX_SHARED_MEMORY_OBJECTS
   :header: API, Supported
   :widths: 50,10

    :c:func:`mmap`, yes
    :c:func:`munmap`, yes
    :c:func:`shm_open`, yes
    :c:func:`shm_unlink`, yes

.. doxygengroup:: posix_option_shared_memory_objects
   :project: posix

.. _posix_option_synchronized_io:

_POSIX_SYNCHRONIZED_IO
++++++++++++++++++++++

Since Zephyr does not yet support Asynchronous I/O, all I/O is, in fact, synchronous.
The functions below are provided for linking only and report success without performing
any actions :ref:`†<posix_undefined_behaviour>`.

Enable this option with :kconfig:option:`CONFIG_POSIX_SYNCHRONIZED_IO`.

.. csv-table:: _POSIX_SYNCHRONIZED_IO
   :header: API, Supported
   :widths: 50,10

    :c:func:`fdatasync`,yes
    :c:func:`fsync`,yes
    :c:func:`msync`,yes

.. doxygengroup:: posix_option_synchronized_io
   :project: posix

.. _posix_option_thread_attr_stackaddr:

_POSIX_THREAD_ATTR_STACKADDR
++++++++++++++++++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_ATTR_STACKADDR`.

``pthread_attr_getstack()`` and ``pthread_attr_setstack()`` also require
:ref:`_POSIX_THREAD_ATTR_STACKSIZE <posix_option_thread_attr_stacksize>` /
:kconfig:option:`CONFIG_POSIX_THREAD_ATTR_STACKSIZE`.

IEEE 1003.1-2017 removed ``pthread_attr_getstackaddr()`` and
``pthread_attr_setstackaddr()`` in favour of the combined stack APIs.

.. csv-table:: _POSIX_THREAD_ATTR_STACKADDR
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_attr_getstack`,yes
    :c:func:`pthread_attr_setstack`,yes

.. doxygengroup:: posix_option_thread_attr_stackaddr
   :project: posix

.. _posix_option_thread_attr_stacksize:

_POSIX_THREAD_ATTR_STACKSIZE
++++++++++++++++++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_ATTR_STACKSIZE`.

.. csv-table:: _POSIX_THREAD_ATTR_STACKSIZE
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_attr_getstacksize`,yes
    :c:func:`pthread_attr_setstacksize`,yes

.. doxygengroup:: posix_option_thread_attr_stacksize
   :project: posix

.. _posix_option_thread_cputime:

_POSIX_THREAD_CPUTIME
+++++++++++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_CPUTIME`.

.. csv-table:: _POSIX_THREAD_CPUTIME
   :header: API, Supported
   :widths: 50,10

    CLOCK_THREAD_CPUTIME_ID,yes
    :c:func:`pthread_getcpuclockid`,yes :ref:`†<posix_undefined_behaviour>`

.. doxygengroup:: posix_option_thread_cputime
   :project: posix

.. _posix_option_thread_prio_inherit:

_POSIX_THREAD_PRIO_INHERIT
++++++++++++++++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_PRIO_INHERIT`.

.. csv-table:: _POSIX_THREAD_PRIO_INHERIT
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_mutexattr_getprotocol`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pthread_mutexattr_setprotocol`,yes :ref:`†<posix_undefined_behaviour>`

.. doxygengroup:: posix_option_thread_prio_inherit
   :project: posix

.. _posix_option_thread_prio_protect:

_POSIX_THREAD_PRIO_PROTECT
++++++++++++++++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_PRIO_PROTECT`.

.. csv-table:: _POSIX_THREAD_PRIO_PROTECT
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_mutex_getprioceiling`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pthread_mutex_setprioceiling`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pthread_mutexattr_getprioceiling`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pthread_mutexattr_getprotocol`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pthread_mutexattr_setprioceiling`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`pthread_mutexattr_setprotocol`,yes :ref:`†<posix_undefined_behaviour>`

.. doxygengroup:: posix_option_thread_prio_protect
   :project: posix

.. _posix_option_thread_priority_scheduling:

_POSIX_THREAD_PRIORITY_SCHEDULING
+++++++++++++++++++++++++++++++++

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_PRIORITY_SCHEDULING`.

.. csv-table:: _POSIX_THREAD_PRIORITY_SCHEDULING
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_attr_getinheritsched`,yes
    :c:func:`pthread_attr_getschedpolicy`,yes
    :c:func:`pthread_attr_getscope`,yes
    :c:func:`pthread_attr_setinheritsched`,yes
    :c:func:`pthread_attr_setschedpolicy`,yes
    :c:func:`pthread_attr_setscope`,yes
    :c:func:`pthread_getschedparam`,yes
    :c:func:`pthread_setschedparam`,yes
    :c:func:`pthread_setschedprio`,yes

.. doxygengroup:: posix_option_thread_priority_scheduling
   :project: posix

.. _posix_option_thread_safe_functions:

_POSIX_THREAD_SAFE_FUNCTIONS
++++++++++++++++++++++++++++

In Zephyr, ``_POSIX_THREAD_SAFE_FUNCTIONS`` is defined when both
:kconfig:option:`CONFIG_POSIX_C_LANG_SUPPORT_R` and
:kconfig:option:`CONFIG_POSIX_FILE_SYSTEM_R` are enabled. The
``POSIX_FILE_LOCKING`` option group is not yet implemented.

.. csv-table:: _POSIX_THREAD_SAFE_FUNCTIONS
    :header: API, Supported
    :widths: 50,10

    :c:func:`asctime_r`, yes
    :c:func:`ctime_r`, yes (UTC timezone only)
    :c:func:`flockfile`,
    :c:func:`ftrylockfile`,
    :c:func:`funlockfile`,
    :c:func:`getc_unlocked`,
    :c:func:`getchar_unlocked`,
    :c:func:`gmtime_r`, yes
    :c:func:`localtime_r`, yes (UTC timezone only)
    :c:func:`putc_unlocked`,
    :c:func:`putchar_unlocked`,
    :c:func:`rand_r`, yes
    :c:func:`readdir_r`, yes
    :c:func:`strerror_r`, yes
    :c:func:`strtok_r`, yes

.. doxygengroup:: posix_option_thread_safe_functions
   :project: posix

.. _posix_option_timeouts:

_POSIX_TIMEOUTS
+++++++++++++++

``_POSIX_TIMEOUTS`` is a POSIX **feature-test macro**, not a Zephyr Option Group. A value
other than -1 means the implementation provides *timed variants* of some otherwise-blocking
APIs (an absolute ``timespec`` deadline instead of waiting indefinitely).

In Zephyr, ``_POSIX_TIMEOUTS`` is defined when :kconfig:option:`CONFIG_POSIX_TIMERS` is
enabled. Each timed API is implemented and tested from the Option Group that owns the
corresponding base function:

- ``pthread_mutex_timedlock()`` — :ref:`POSIX_THREADS_BASE<posix_option_group_threads_base>`
- ``sem_timedwait()`` — :ref:`POSIX_SEMAPHORES<posix_option_group_semaphores>`
- ``mq_timedsend()`` / ``mq_timedreceive()`` — :ref:`_POSIX_MESSAGE_PASSING<posix_option_message_passing>`
- ``pthread_rwlock_timedrdlock()`` / ``pthread_rwlock_timedwrlock()`` —
  :ref:`POSIX_RW_LOCKS<posix_option_group_rw_locks>`

.. _posix_option_group_xsi_streams:
.. _posix_option_xopen_streams:

_XOPEN_STREAMS
++++++++++++++

With the exception of ``ioctl()``, functions in the ``_XOPEN_STREAMS`` option group are not
implemented in Zephyr but are provided so that conformant applications can still link.
Unimplemented functions in this option group will fail, setting ``errno`` to ``ENOSYS``
:ref:`†<posix_undefined_behaviour>`.

Enable this option with :kconfig:option:`CONFIG_XSI_STREAMS`.

.. csv-table:: _XOPEN_STREAMS
   :header: API, Supported
   :widths: 50,10

    :c:func:`fattach`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`fdetach`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`getmsg`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`getpmsg`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`ioctl`, yes
    :c:func:`isastream`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`putmsg`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`putpmsg`, yes :ref:`†<posix_undefined_behaviour>`

.. _Subprofiling Considerations:
    https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html

