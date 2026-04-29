.. _posix_option_group_semaphores:

POSIX_SEMAPHORES
================

Enable this option group with :kconfig:option:`CONFIG_POSIX_SEMAPHORES`.

.. csv-table:: POSIX_SEMAPHORES
   :header: API, Supported
   :widths: 50,10

    :c:func:`sem_close`,yes
    :c:func:`sem_destroy`,yes
    :c:func:`sem_getvalue`,yes
    :c:func:`sem_init`,yes
    :c:func:`sem_open`,yes
    :c:func:`sem_post`,yes
    :c:func:`sem_trywait`,yes
    :c:func:`sem_unlink`,yes
    :c:func:`sem_wait`,yes

