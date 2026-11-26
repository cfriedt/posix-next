.. _posix_option_job_control:

_POSIX_JOB_CONTROL
==================

The ``_POSIX_JOB_CONTROL`` Option does not add any interfaces of its own; it indicates that
each process group of a session may be stopped, resumed, and given exclusive access to the
session's controlling terminal. Beyond process groups (provided by
:ref:`POSIX_MULTI_PROCESS <posix_option_group_multi_process>` and
:ref:`POSIX_SIGNALS <posix_option_group_signals>`), the option requires the
terminal control functions :c:func:`tcdrain`, :c:func:`tcflush`, :c:func:`tcgetpgrp`,
:c:func:`tcsendbreak`, :c:func:`tcsetattr`, and :c:func:`tcsetpgrp` (see
:ref:`POSIX_DEVICE_SPECIFIC <posix_option_group_device_specific>` and
:ref:`POSIX_JOB_CONTROL <posix_option_group_job_control>`). Zephyr has no controlling
terminal, so the option is unsupported.
