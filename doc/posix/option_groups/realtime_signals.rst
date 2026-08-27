.. _posix_option_group_realtime_signals:

POSIX_REALTIME_SIGNALS
======================

Enable this option group with :kconfig:option:`CONFIG_POSIX_REALTIME_SIGNALS`.

.. csv-table:: POSIX_REALTIME_SIGNALS
   :header: API, Supported
   :widths: 50,10

    :c:func:`sigqueue`,yes
    :c:func:`sigtimedwait`,yes
    :c:func:`sigwaitinfo`,yes

..
   this link is "deprecated" - mainly left here so that older links still work

.. doxygengroup:: posix_option_group_realtime_signals
   :project: posix

