.. _posix_option_synchronized_io:

_POSIX_SYNCHRONIZED_IO
======================

Since Zephyr does not yet support Asynchronous I/O, all I/O is, in fact, synchronous.
The functions below are provided for linking only and report success without performing
any actions :ref:`†<posix_undefined_behaviour>`.

Enable this option with :kconfig:option:`CONFIG_POSIX_SYNCHRONIZED_IO`.

.. csv-table:: _POSIX_SYNCHRONIZED_IO
   :header: API, Supported
   :widths: 50,10

    :c:func:`fdatasync`,yes
    :c:func:`fsync`,yes
    :c:func:`msync`,yes

.. doxygengroup:: posix_option_synchronized_io
   :project: posix
