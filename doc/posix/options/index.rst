.. _posix_options:

POSIX Options
#############

For the conformance summary (mandatory, optional, and unsupported symbols), see
:ref:`POSIX Conformance <posix_conformance>`. For bundled API groups, see
:ref:`POSIX Option Groups <posix_option_groups>`.

Options that correspond to Option Groups
----------------------------------------

These Options are synonymous with an official :ref:`Subprofiling Option Group
<posix_option_groups>`; see the linked Option Group page for per-API detail.

.. toctree::
   :hidden:

   barriers
   clock_selection
   mapped_files
   memory_protection
   realtime_signals
   semaphores
   spin_locks
   threads
   timers
   xopen_realtime
   xopen_realtime_threads

- :ref:`_POSIX_BARRIERS <posix_option_group_barriers>`
- :ref:`_POSIX_CLOCK_SELECTION <posix_option_group_clock_selection>`
- :ref:`_POSIX_MAPPED_FILES <posix_option_group_mapped_files>`
- :ref:`_POSIX_MEMORY_PROTECTION <posix_option_group_memory_protection>`
- :ref:`_POSIX_REALTIME_SIGNALS <posix_option_group_realtime_signals>`
- :ref:`_POSIX_REGEXP <posix_option_group_regexp>`
- :ref:`_POSIX_SEMAPHORES <posix_option_group_semaphores>`
- :ref:`_POSIX_SPIN_LOCKS <posix_option_group_spin_locks>`
- :ref:`_POSIX_THREADS <posix_option_group_threads_base>`
- :ref:`_POSIX_TIMERS <posix_option_group_timers>`
- :ref:`_XOPEN_REALTIME <posix_option_group_xsi_realtime>`
- :ref:`_XOPEN_REALTIME_THREADS <posix_option_group_xsi_realtime_threads>`

Option Group-like Options
-------------------------

These Options provide interfaces of their own, but are not official
Subprofiling Option Groups because they are not independent of other Option
Groups. They are documented with the same per-API detail as Option Groups.

.. toctree::
   :maxdepth: 1

   advisory_info
   asynchronous_io
   cputime
   fsync
   memlock
   memlock_range
   message_passing
   priority_scheduling
   shared_memory_objects
   spawn
   synchronized_io
   thread_attr_stackaddr
   thread_attr_stacksize
   thread_cputime
   thread_prio_inherit
   thread_prio_protect
   thread_priority_scheduling
   thread_process_shared
   thread_safe_functions
   timeouts
   xsi_streams

Options without interfaces of their own
---------------------------------------

These Options do not add any interfaces; they indicate the presence of
functionality provided elsewhere.

.. toctree::
   :hidden:

   chown_restricted
   ipv6
   job_control
   monotonic_clock
   posix2
   prioritized_io
   raw_sockets
   saved_ids
   shell
   sporadic_server
   thread_sporadic_server
   vdisable

:ref:`_POSIX_CHOWN_RESTRICTED <posix_option_chown_restricted>`
   Use of :c:func:`chown` is restricted to a process with appropriate privileges.
   Unsupported: Zephyr does not implement file ownership.

:ref:`_POSIX_IPV6 <posix_option_ipv6>`
   Internet Protocol Version 6 is supported by the
   :ref:`POSIX_NETWORKING <posix_option_group_networking>` interfaces. Enable
   with :kconfig:option:`CONFIG_POSIX_IPV6`.

:ref:`_POSIX_JOB_CONTROL <posix_option_job_control>`
   Process groups of a session may be stopped, resumed, and given exclusive access to the
   session's controlling terminal. Unsupported: the required terminal control functions
   are unimplemented.

:ref:`_POSIX_MONOTONIC_CLOCK <posix_option_monotonic_clock>`
   The ``CLOCK_MONOTONIC`` clock is supported by the clock and timer
   interfaces (mandatory since Issue 8). Enable with
   :kconfig:option:`CONFIG_POSIX_MONOTONIC_CLOCK`.

:ref:`_POSIX_PRIORITIZED_IO <posix_option_prioritized_io>`
   Asynchronous I/O requests are processed in priority order - the calling
   thread's priority lowered per request by ``aio_reqprio`` - by the
   :ref:`_POSIX_ASYNCHRONOUS_IO <posix_option_asynchronous_io>` interfaces.
   Enable with :kconfig:option:`CONFIG_POSIX_PRIORITIZED_IO`.

:ref:`_POSIX_RAW_SOCKETS <posix_option_raw_sockets>`
   Raw sockets are supported by the
   :ref:`POSIX_NETWORKING <posix_option_group_networking>` interfaces. Enable
   with :kconfig:option:`CONFIG_POSIX_RAW_SOCKETS`.

:ref:`_POSIX_SAVED_IDS <posix_option_saved_ids>`
   Each process keeps a saved set-user-ID and saved set-group-ID. Unsupported: Zephyr has
   no user or group identities.

:ref:`_POSIX_SHELL <posix_option_shell>`
   :c:func:`system` executes commands with a conformant shell. Unsupported: Zephyr's shell
   is not a POSIX shell.

:ref:`_POSIX_SPORADIC_SERVER <posix_option_sporadic_server>`
   The ``SCHED_SPORADIC`` scheduling policy is accepted by the
   :ref:`_POSIX_PRIORITY_SCHEDULING <posix_option_priority_scheduling>`
   interfaces, though execution-time budgets are not enforced
   :ref:`†<posix_undefined_behaviour>`. Enable with
   :kconfig:option:`CONFIG_POSIX_SPORADIC_SERVER`.

:ref:`_POSIX_THREAD_PROCESS_SHARED <posix_option_thread_process_shared>`
   Mutexes, condition variables, barriers, read-write locks, spin locks and
   semaphores may be shared between processes through memory both of them
   map. Enable with :kconfig:option:`CONFIG_POSIX_THREAD_PROCESS_SHARED`.

:ref:`_POSIX_THREAD_SPORADIC_SERVER <posix_option_thread_sporadic_server>`
   The ``SCHED_SPORADIC`` scheduling policy is accepted by the
   :ref:`_POSIX_THREAD_PRIORITY_SCHEDULING <posix_option_thread_priority_scheduling>`
   interfaces, though execution-time budgets are not enforced
   :ref:`†<posix_undefined_behaviour>`. Enable with
   :kconfig:option:`CONFIG_POSIX_THREAD_SPORADIC_SERVER`.

:ref:`_POSIX_VDISABLE <posix_option_vdisable>`
   Terminal special characters can be disabled via :c:func:`tcsetattr`. Unsupported: the
   general terminal interface is unimplemented.

:ref:`_POSIX2 Shell and Utilities Options <posix_options_posix2>`
   The ``_POSIX2_*`` constants describe the Shell and Utilities volume, which is beyond the
   scope of Zephyr's POSIX support.
