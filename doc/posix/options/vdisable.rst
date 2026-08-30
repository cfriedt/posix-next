.. _posix_option_vdisable:

_POSIX_VDISABLE
===============

The ``_POSIX_VDISABLE`` Option does not add any interfaces; it indicates that terminal
special characters (such as ``VINTR`` and ``VSUSP``) can be disabled via
:c:func:`tcsetattr`. It is unsupported because Zephyr does not implement the general
terminal interface; see :ref:`POSIX_DEVICE_SPECIFIC <posix_option_group_device_specific>`.
