.. _posix_option_prioritized_io:

_POSIX_PRIORITIZED_IO
=====================

This option does not add any interfaces of its own; it makes the
:ref:`_POSIX_ASYNCHRONOUS_IO <posix_option_asynchronous_io>` interfaces process ready
requests in priority order rather than submission order - highest priority first,
first-in-first-out among equal priorities - where a request's priority is the calling
thread's scheduling priority lowered by the control block's ``aio_reqprio`` (0 through
``AIO_PRIO_DELTA_MAX``, one Zephyr priority level per unit). Prioritization applies to
every descriptor served by the request pool; see the
:ref:`asynchronous I/O page <posix_option_asynchronous_io>` for the implementation-defined
behaviour.

Enable this option with :kconfig:option:`CONFIG_POSIX_PRIORITIZED_IO` (enabled by default
with :kconfig:option:`CONFIG_POSIX_ASYNCHRONOUS_IO`; selects
:kconfig:option:`CONFIG_SYS_AIO_PRIORITY`).
