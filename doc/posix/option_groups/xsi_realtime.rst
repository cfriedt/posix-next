.. _posix_option_group_xsi_realtime:

XSI_REALTIME
============

The ``XSI_REALTIME`` option group indicates that the :ref:`_POSIX_FSYNC<posix_option_fsync>`,
:ref:`_POSIX_MEMLOCK<posix_option_memlock>`,
:ref:`_POSIX_MEMLOCK_RANGE<posix_option_memlock_range>`,
:ref:`_POSIX_MESSAGE_PASSING<posix_option_message_passing>`,
:ref:`_POSIX_PRIORITY_SCHEDULING<posix_option_priority_scheduling>`,
:ref:`_POSIX_SHARED_MEMORY_OBJECTS<posix_option_shared_memory_objects>`, and
:ref:`_POSIX_SYNCHRONIZED_IO<posix_option_synchronized_io>` options are enabled.

Enable this option group with :kconfig:option:`CONFIG_XSI_REALTIME`.

When this option group is enabled, the ``_XOPEN_REALTIME`` feature test macro will be defined to a
value other than -1.

