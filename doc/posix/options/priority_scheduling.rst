.. _posix_option_priority_scheduling:

_POSIX_PRIORITY_SCHEDULING
==========================

The process-addressed scheduling functions ``sched_getparam()``, ``sched_getscheduler()``,
``sched_rr_get_interval()``, ``sched_setparam()``, and ``sched_setscheduler()`` are not yet
wired to Zephyr's process support (:ref:`POSIX_MULTI_PROCESS <posix_option_group_multi_process>`)
and fail, setting ``errno`` to ``ENOSYS``:ref:`†<posix_undefined_behaviour>`.

Enable this option with :kconfig:option:`CONFIG_POSIX_PRIORITY_SCHEDULING`.

.. csv-table:: _POSIX_PRIORITY_SCHEDULING
   :header: API, Supported
   :widths: 50,10

    :c:func:`sched_get_priority_max`,yes
    :c:func:`sched_get_priority_min`,yes
    :c:func:`sched_getparam`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sched_getscheduler`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sched_rr_get_interval`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sched_setparam`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`sched_setscheduler`,yes :ref:`†<posix_undefined_behaviour>`

.. doxygengroup:: posix_option_priority_scheduling
   :project: posix

