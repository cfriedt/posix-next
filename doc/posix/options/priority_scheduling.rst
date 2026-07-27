.. _posix_option_priority_scheduling:

_POSIX_PRIORITY_SCHEDULING
==========================

As processes are not yet supported in Zephyr, the process-scoped functions operate on the
calling thread: a ``pid`` of 0 and the value returned by ``getpid()`` both designate the
current thread (as on Linux); any other ``pid`` fails with ``ESRCH``.

Enable this option with :kconfig:option:`CONFIG_POSIX_PRIORITY_SCHEDULING`.

.. csv-table:: _POSIX_PRIORITY_SCHEDULING
   :header: API, Supported
   :widths: 50,10

    :c:func:`sched_get_priority_max`,yes
    :c:func:`sched_get_priority_min`,yes
    :c:func:`sched_getparam`,yes
    :c:func:`sched_getscheduler`,yes
    :c:func:`sched_rr_get_interval`,yes
    :c:func:`sched_setparam`,yes
    :c:func:`sched_setscheduler`,yes

.. doxygengroup:: posix_option_priority_scheduling
   :project: posix

