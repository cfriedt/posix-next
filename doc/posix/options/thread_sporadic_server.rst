.. _posix_option_thread_sporadic_server:

_POSIX_THREAD_SPORADIC_SERVER
=============================

This option does not add any interfaces of its own; it makes the ``SCHED_SPORADIC`` scheduling
policy and the ``sched_ss_*`` members of ``struct sched_param`` available to the scheduling
interfaces of
:ref:`_POSIX_THREAD_PRIORITY_SCHEDULING <posix_option_thread_priority_scheduling>`.

Zephyr accepts ``SCHED_SPORADIC`` and validates the sporadic server parameters, but does not
enforce execution-time budgets: a thread scheduled under ``SCHED_SPORADIC`` executes as if
scheduled under ``SCHED_RR`` at ``sched_priority`` :ref:`†<posix_undefined_behaviour>`. See
:ref:`posix_sporadic_server` for details and rationale.

Enable this option with :kconfig:option:`CONFIG_POSIX_THREAD_SPORADIC_SERVER`, or enable the
:ref:`XSI_ADVANCED_REALTIME_THREADS <posix_option_group_xsi_advanced_realtime_threads>` option
group with :kconfig:option:`CONFIG_XSI_ADVANCED_REALTIME_THREADS`.

.. doxygengroup:: posix_option_thread_sporadic_server
   :project: posix
