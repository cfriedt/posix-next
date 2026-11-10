.. _posix_option_chown_restricted:

_POSIX_CHOWN_RESTRICTED
=======================

The ``_POSIX_CHOWN_RESTRICTED`` Option does not add any interfaces; it indicates that use of
:c:func:`chown` is restricted to a process with appropriate privileges. Zephyr is a single-user
system whose only identity is the privileged user, so every :c:func:`chown` call has
appropriate privileges and may give a file to any owner, and
:c:func:`pathconf` reports ``_PC_CHOWN_RESTRICTED`` for every file; see
:ref:`POSIX_FILE_ATTRIBUTES <posix_option_group_file_attributes>`.
