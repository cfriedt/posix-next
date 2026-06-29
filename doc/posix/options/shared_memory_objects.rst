.. _posix_option_shared_memory_objects:

_POSIX_SHARED_MEMORY_OBJECTS
============================

Enable this option with :kconfig:option:`CONFIG_POSIX_SHARED_MEMORY_OBJECTS`.

.. csv-table:: _POSIX_SHARED_MEMORY_OBJECTS
   :header: API, Supported
   :widths: 50,10

    :c:func:`mmap`, yes
    :c:func:`munmap`, yes
    :c:func:`shm_open`, yes
    :c:func:`shm_unlink`, yes

.. doxygengroup:: posix_option_shared_memory_objects
   :project: posix

