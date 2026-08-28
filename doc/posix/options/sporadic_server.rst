.. _posix_option_sporadic_server:

_POSIX_SPORADIC_SERVER
======================

This option does not add any interfaces of its own; it makes the ``SCHED_SPORADIC`` scheduling
policy and the ``sched_ss_*`` members of ``struct sched_param`` available to the process
scheduling interfaces of :ref:`_POSIX_PRIORITY_SCHEDULING <posix_option_priority_scheduling>`.

Zephyr accepts ``SCHED_SPORADIC`` in ``sched_get_priority_min()`` and
``sched_get_priority_max()``, but does not enforce execution-time budgets; the
process-addressed scheduling functions (``sched_setparam()`` and ``sched_setscheduler()``) are
not yet wired to Zephyr's process support and fail, setting ``errno`` to ``ENOSYS``
:ref:`†<posix_undefined_behaviour>`. See :ref:`posix_sporadic_server` for details and
rationale.

Enable this option with :kconfig:option:`CONFIG_POSIX_SPORADIC_SERVER`, or enable the
:ref:`XSI_ADVANCED_REALTIME <posix_option_group_xsi_advanced_realtime>` option group with
:kconfig:option:`CONFIG_XSI_ADVANCED_REALTIME`.
