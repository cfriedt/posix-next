POSIX Options
#############

.. _posix_options:

POSIX Options
=============

POSIX options are symbolic constants (typically ``_POSIX_*`` or ``_XOPEN_*``) that denote
optional or mandatory feature sets. Supported options are documented below; unsupported symbols
are listed in the summary tables only. See also :ref:`POSIX Conformance <posix_conformance>` for
the full conformance matrix.

The option list follows `IEEE 1003.1`_ (Open Group Base Specifications), with primary alignment
to Issue 7 (200809L) and notes where Issue 8 (202405L) changed requirements. Options removed
from the specification (for example POSIX tracing in IEEE 1003.13) are not listed here.

.. note::
   In Issue 7, ``_POSIX_MONOTONIC_CLOCK`` was an optional option. Issue 8 promoted it to a
   mandatory option alongside the other 200809L baseline interfaces.

.. _IEEE 1003.1: https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/V1_chap02.html#tag_02_01_03_01

Mandatory Options
-----------------

These options are required for POSIX system-interface conformance in Issue 7 (200809L).

.. csv-table:: Mandatory POSIX Options
   :header: Symbol, Support, Remarks
   :widths: 50, 10, 50

    :ref:`_POSIX_ASYNCHRONOUS_IO<posix_option_asynchronous_io>`, 200809L, :kconfig:option:`CONFIG_POSIX_ASYNCHRONOUS_IO` :ref:`†<posix_undefined_behaviour>`
    :ref:`_POSIX_BARRIERS<posix_option_barriers>`, 200809L, :kconfig:option:`CONFIG_POSIX_BARRIERS`
    :ref:`_POSIX_CLOCK_SELECTION<posix_option_clock_selection>`, 200809L, :kconfig:option:`CONFIG_POSIX_CLOCK_SELECTION`
    :ref:`_POSIX_MAPPED_FILES<posix_option_mapped_files>`, 200809L, :kconfig:option:`CONFIG_POSIX_MAPPED_FILES`
    :ref:`_POSIX_MEMORY_PROTECTION<posix_option_memory_protection>`, 200809L, :kconfig:option:`CONFIG_POSIX_MEMORY_PROTECTION` :ref:`†<posix_undefined_behaviour>`
    :ref:`_POSIX_READER_WRITER_LOCKS<posix_option_reader_writer_locks>`, 200809L, :kconfig:option:`CONFIG_POSIX_RW_LOCKS`
    :ref:`_POSIX_REALTIME_SIGNALS<posix_option_realtime_signals>`, 200809L, :kconfig:option:`CONFIG_POSIX_REALTIME_SIGNALS`
    :ref:`_POSIX_SEMAPHORES<posix_option_semaphores>`, 200809L, :kconfig:option:`CONFIG_POSIX_SEMAPHORES`
    :ref:`_POSIX_SPIN_LOCKS<posix_option_spin_locks>`, 200809L, :kconfig:option:`CONFIG_POSIX_SPIN_LOCKS`
    :ref:`_POSIX_THREAD_SAFE_FUNCTIONS<posix_option_thread_safe_functions>`, 200809L, :kconfig:option:`CONFIG_POSIX_C_LANG_SUPPORT_R` and :kconfig:option:`CONFIG_POSIX_FILE_SYSTEM_R` and :kconfig:option:`CONFIG_POSIX_FILE_LOCKING`
    :ref:`_POSIX_THREADS<posix_option_threads>`, 200809L, :kconfig:option:`CONFIG_POSIX_THREADS`
    :ref:`_POSIX_TIMEOUTS<posix_option_timeouts>`, 200809L, :kconfig:option:`CONFIG_POSIX_TIMERS`
    :ref:`_POSIX_TIMERS<posix_option_timers>`, 200809L, :kconfig:option:`CONFIG_POSIX_TIMERS`

Optional Options
----------------

.. csv-table:: Optional POSIX Options
   :header: Symbol, Support, Remarks
   :widths: 50, 10, 50

    _POSIX_ADVISORY_INFO, -1,
    :ref:`_POSIX_CPUTIME<posix_option_cputime>`, 200809L, :kconfig:option:`CONFIG_POSIX_CPUTIME`
    _POSIX_DEVICE_CONTROL, -1, Issue 8 only
    :ref:`_POSIX_FSYNC<posix_option_fsync>`, 200809L, :kconfig:option:`CONFIG_POSIX_FSYNC`
    :ref:`_POSIX_IPV6<posix_option_ipv6>`, 200809L, :kconfig:option:`CONFIG_POSIX_IPV6`
    :ref:`_POSIX_MEMLOCK <posix_option_memlock>`, 200809L, :kconfig:option:`CONFIG_POSIX_MEMLOCK` :ref:`†<posix_undefined_behaviour>`
    :ref:`_POSIX_MEMLOCK_RANGE <posix_option_memlock_range>`, 200809L, :kconfig:option:`CONFIG_POSIX_MEMLOCK_RANGE`
    :ref:`_POSIX_MESSAGE_PASSING<posix_option_message_passing>`, 200809L, :kconfig:option:`CONFIG_POSIX_MESSAGE_PASSING`
    :ref:`_POSIX_MONOTONIC_CLOCK<posix_option_monotonic_clock>`, 200809L, :kconfig:option:`CONFIG_POSIX_MONOTONIC_CLOCK`
    _POSIX_PRIORITIZED_IO, -1,
    :ref:`_POSIX_PRIORITY_SCHEDULING<posix_option_priority_scheduling>`, 200809L, :kconfig:option:`CONFIG_POSIX_PRIORITY_SCHEDULING`
    :ref:`_POSIX_RAW_SOCKETS<posix_option_raw_sockets>`, 200809L, :kconfig:option:`CONFIG_POSIX_RAW_SOCKETS`
    :ref:`_POSIX_SHARED_MEMORY_OBJECTS <posix_option_shared_memory_objects>`, 200809L, :kconfig:option:`CONFIG_POSIX_SHARED_MEMORY_OBJECTS`
    _POSIX_SPAWN, -1, :ref:`†<posix_undefined_behaviour>`
    _POSIX_SPORADIC_SERVER, -1, :ref:`†<posix_undefined_behaviour>`
    :ref:`_POSIX_SYNCHRONIZED_IO <posix_option_synchronized_io>`, 200809L, :kconfig:option:`CONFIG_POSIX_SYNCHRONIZED_IO`
    :ref:`_POSIX_THREAD_ATTR_STACKADDR<posix_option_thread_attr_stackaddr>`, 200809L, :kconfig:option:`CONFIG_POSIX_THREAD_ATTR_STACKADDR`
    :ref:`_POSIX_THREAD_ATTR_STACKSIZE<posix_option_thread_attr_stacksize>`, 200809L, :kconfig:option:`CONFIG_POSIX_THREAD_ATTR_STACKSIZE`
    :ref:`_POSIX_THREAD_CPUTIME <posix_option_thread_cputime>`, 200809L, :kconfig:option:`CONFIG_POSIX_CPUTIME`
    :ref:`_POSIX_THREAD_PRIO_INHERIT <posix_option_thread_prio_inherit>`, 200809L, :kconfig:option:`CONFIG_POSIX_THREAD_PRIO_INHERIT`
    :ref:`_POSIX_THREAD_PRIO_PROTECT <posix_option_thread_prio_protect>`, 200809L, :kconfig:option:`CONFIG_POSIX_THREAD_PRIO_PROTECT`
    :ref:`_POSIX_THREAD_PRIORITY_SCHEDULING <posix_option_thread_priority_scheduling>`, 200809L, :kconfig:option:`CONFIG_POSIX_THREAD_PRIORITY_SCHEDULING`
    _POSIX_THREAD_PROCESS_SHARED, -1,
    _POSIX_THREAD_ROBUST_PRIO_INHERIT, -1, Issue 8 only
    _POSIX_THREAD_ROBUST_PRIO_PROTECT, -1, Issue 8 only
    _POSIX_THREAD_SPORADIC_SERVER, -1,
    _POSIX_TYPED_MEMORY_OBJECTS, -1,
    _XOPEN_CRYPT, -1,
    :ref:`_XOPEN_REALTIME <posix_option_xopen_realtime>`, 700, :kconfig:option:`CONFIG_XSI_REALTIME`
    :ref:`_XOPEN_REALTIME_THREADS <posix_option_xopen_realtime_threads>`, 700, :kconfig:option:`CONFIG_XSI_REALTIME_THREADS`
    :ref:`_XOPEN_STREAMS<posix_option_xopen_streams>`, 200809L, :kconfig:option:`CONFIG_XSI_STREAMS`
    _XOPEN_UNIX, 700, :kconfig:option:`CONFIG_XSI`

Option Details
--------------

.. toctree::
   :maxdepth: 1

   asynchronous_io
   barriers
   clock_selection
   cputime
   fsync
   ipv6
   mapped_files
   memlock
   memlock_range
   memory_protection
   message_passing
   monotonic_clock
   priority_scheduling
   raw_sockets
   realtime_signals
   semaphores
   shared_memory_objects
   spin_locks
   synchronized_io
   thread_attr_stackaddr
   thread_attr_stacksize
   thread_cputime
   thread_prio_inherit
   thread_prio_protect
   thread_priority_scheduling
   thread_safe_functions
   threads
   timeouts
   timers
   xopen_realtime
   xopen_realtime_threads
   xsi_streams
