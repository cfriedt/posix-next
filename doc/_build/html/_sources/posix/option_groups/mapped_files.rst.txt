.. _posix_option_group_mapped_files:

POSIX_MAPPED_FILES
==================

Enable this option group with :kconfig:option:`CONFIG_POSIX_MAPPED_FILES`.

.. csv-table:: POSIX_MAPPED_FILES
   :header: API, Supported
   :widths: 50,10

    :c:func:`mmap`,yes
    :c:func:`msync`,yes
    :c:func:`munmap`,yes

.. doxygengroup:: posix_option_group_mapped_files
   :project: posix

