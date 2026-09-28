.. _posix_option_group_mapped_files:

POSIX_MAPPED_FILES
==================

Enable this option group with :kconfig:option:`CONFIG_POSIX_MAPPED_FILES`.

.. csv-table:: POSIX_MAPPED_FILES
   :header: API, Supported
   :widths: 50,10

    :c:func:`mmap`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`munmap`,yes :ref:`†<posix_undefined_behaviour>`

See :ref:`posix_mapped_files_design` for implementation details.

.. doxygengroup:: posix_option_group_mapped_files
   :project: posix

