.. _posix_option_message_passing:

_POSIX_MESSAGE_PASSING
======================

Enable this option with :kconfig:option:`CONFIG_POSIX_MESSAGE_PASSING`.

.. csv-table:: _POSIX_MESSAGE_PASSING
   :header: API, Supported
   :widths: 50,10

    :c:func:`mq_close`,yes
    :c:func:`mq_getattr`,yes
    :c:func:`mq_notify`,yes
    :c:func:`mq_open`,yes
    :c:func:`mq_receive`,yes
    :c:func:`mq_send`,yes
    :c:func:`mq_setattr`,yes
    :c:func:`mq_unlink`,yes

