.. _posix_option_shared_memory_objects:

_POSIX_SHARED_MEMORY_OBJECTS
============================

Enable this option with :kconfig:option:`CONFIG_POSIX_SHARED_MEMORY_OBJECTS`.

.. csv-table:: _POSIX_SHARED_MEMORY_OBJECTS
   :header: API, Supported
   :widths: 50,10

    :c:func:`mmap`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`munmap`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`shm_open`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`shm_unlink`,yes :ref:`†<posix_undefined_behaviour>`

See :ref:`posix_mapped_files_design` for implementation details.

.. doxygengroup:: posix_option_shared_memory_objects
   :project: posix

