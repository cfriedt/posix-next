.. _posix_option_vdisable:

_POSIX_VDISABLE
===============

The ``_POSIX_VDISABLE`` Option does not add any interfaces; it indicates that terminal
special characters (such as ``VINTR`` and ``VSUSP``) can be disabled via
:c:func:`tcsetattr` by setting them to the ``_POSIX_VDISABLE`` value, which is ``'\\0'``. The
console terminal keeps the control characters it is given, and :c:func:`fpathconf` reports
``_PC_VDISABLE`` for a terminal descriptor; see
:ref:`POSIX_DEVICE_SPECIFIC <posix_option_group_device_specific>`.
