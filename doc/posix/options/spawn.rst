.. _posix_option_spawn:

_POSIX_SPAWN
============

A spawned child is a new process running a prelinked executable image: the
``path`` (or ``file``) argument is resolved by the application-provided
``posix_spawn_image_lookup()`` registry rather than loaded from a file
system.

Enable this option with :kconfig:option:`CONFIG_POSIX_SPAWN`.

.. csv-table:: _POSIX_SPAWN
   :header: API, Supported
   :widths: 50,10

    :c:func:`posix_spawn`,yes
    :c:func:`posix_spawn_file_actions_addclose`,yes
    :c:func:`posix_spawn_file_actions_adddup2`,yes
    :c:func:`posix_spawn_file_actions_addopen`,yes
    :c:func:`posix_spawn_file_actions_destroy`,yes
    :c:func:`posix_spawn_file_actions_init`,yes
    :c:func:`posix_spawnattr_destroy`,yes
    :c:func:`posix_spawnattr_getflags`,yes
    :c:func:`posix_spawnattr_getpgroup`,yes
    :c:func:`posix_spawnattr_getschedparam`,yes
    :c:func:`posix_spawnattr_getschedpolicy`,yes
    :c:func:`posix_spawnattr_getsigdefault`,yes
    :c:func:`posix_spawnattr_getsigmask`,yes
    :c:func:`posix_spawnattr_init`,yes
    :c:func:`posix_spawnattr_setflags`,yes
    :c:func:`posix_spawnattr_setpgroup`,yes
    :c:func:`posix_spawnattr_setschedparam`,yes
    :c:func:`posix_spawnattr_setschedpolicy`,yes
    :c:func:`posix_spawnattr_setsigdefault`,yes
    :c:func:`posix_spawnattr_setsigmask`,yes
    :c:func:`posix_spawnp`,yes

.. doxygengroup:: posix_option_spawn
   :project: posix
