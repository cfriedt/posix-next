.. _posix_option_saved_ids:

_POSIX_SAVED_IDS
================

The ``_POSIX_SAVED_IDS`` Option does not add any interfaces; it indicates that each process
keeps a saved set-user-ID and saved set-group-ID. It is unsupported because Zephyr is a
single-user system without user or group identities; see
:ref:`POSIX_USER_GROUPS <posix_option_group_user_groups>`.
