.. _posix_option_priority_scheduling:

_POSIX_PRIORITY_SCHEDULING
==========================

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

.. doxygengroup:: posix_option_priority_scheduling
   :project: posix

