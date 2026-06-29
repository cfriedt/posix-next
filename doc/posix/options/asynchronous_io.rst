.. _posix_option_asynchronous_io:

_POSIX_ASYNCHRONOUS_IO
======================

Functions part of the ``_POSIX_ASYNCHRONOUS_IO`` Option are not implemented in Zephyr but are
provided so that conformant applications can still link. These functions will fail, setting
``errno`` to ``ENOSYS``:ref:`†<posix_undefined_behaviour>`.

Enable this option with :kconfig:option:`CONFIG_POSIX_ASYNCHRONOUS_IO`.

.. csv-table:: _POSIX_ASYNCHRONOUS_IO
   :header: API, Supported
   :widths: 50,10

    :c:func:`aio_cancel`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`aio_error`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`aio_fsync`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`aio_read`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`aio_return`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`aio_suspend`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`aio_write`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`lio_listio`,yes :ref:`†<posix_undefined_behaviour>`

