.. _posix_option_group_xsi_streams:
.. _posix_option_xopen_streams:

_XOPEN_STREAMS
==============

With the exception of ``ioctl()``, functions in the ``_XOPEN_STREAMS`` option group are not
implemented in Zephyr but are provided so that conformant applications can still link.
Unimplemented functions in this option group will fail, setting ``errno`` to ``ENOSYS``
:ref:`†<posix_undefined_behaviour>`.

Enable this option with :kconfig:option:`CONFIG_XSI_STREAMS`.

.. csv-table:: _XOPEN_STREAMS
   :header: API, Supported
   :widths: 50,10

    :c:func:`fattach`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`fdetach`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`getmsg`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`getpmsg`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`ioctl`, yes
    :c:func:`isastream`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`putmsg`, yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`putpmsg`, yes :ref:`†<posix_undefined_behaviour>`

