.. _posix_option_synchronized_io:

_POSIX_SYNCHRONIZED_IO
======================

The functions below complete synchronously; synchronization beyond what the underlying
file system already guarantees may be a no-op :ref:`†<posix_undefined_behaviour>`.
Asynchronous variants are provided by
:ref:`_POSIX_ASYNCHRONOUS_IO <posix_option_asynchronous_io>` (see :c:func:`aio_fsync`).

Enable this option with :kconfig:option:`CONFIG_POSIX_SYNCHRONIZED_IO`.

.. csv-table:: _POSIX_SYNCHRONIZED_IO
   :header: API, Supported
   :widths: 50,10

    :c:func:`fdatasync`,yes
    :c:func:`fsync`,yes
    :c:func:`msync`,yes

.. doxygengroup:: posix_option_synchronized_io
   :project: posix
