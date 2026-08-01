.. _posix_option_asynchronous_io:

_POSIX_ASYNCHRONOUS_IO
======================

Asynchronous I/O is backed by the OS-managed request pool
(:kconfig:option:`CONFIG_SYS_AIO`): submitted operations are performed by a dedicated kernel
service queue that waits for descriptor readiness where the descriptor has a waitable
readiness condition (e.g. sockets and eventfds) and performs the operation directly
otherwise (e.g. regular files). Completion is observed with :c:func:`aio_error`,
:c:func:`aio_suspend`, and :c:func:`aio_return`, or announced by the control block's
``aio_sigevent`` (``SIGEV_SIGNAL`` deliveries carry an ``SI_ASYNCIO`` code;
``SIGEV_THREAD`` functions run in a fresh detached thread). :c:func:`lio_listio` with
``LIO_NOWAIT`` supports a list-completion ``sigevent`` that fires once, when the last
operation of the list completes.

Notes on implementation-defined behaviour:

* The buffer named by a control block must remain valid until the operation is retrieved
  with :c:func:`aio_return`; closing a descriptor with operations in flight completes them
  with ``EBADF``, so cancel before closing.
* :c:func:`aio_fsync` synchronizes data and metadata together: ``O_DSYNC`` and ``O_SYNC``
  are equivalent.
* With :kconfig:option:`CONFIG_POSIX_PRIORITIZED_IO` (enabled by default; selects
  :kconfig:option:`CONFIG_SYS_AIO_PRIORITY`), ``_POSIX_PRIORITIZED_IO`` is defined and ready
  operations are performed in priority order - the calling thread's scheduling priority
  lowered by ``aio_reqprio`` (0 through ``AIO_PRIO_DELTA_MAX``, one Zephyr priority level per
  unit), first-in-first-out among equal priorities - for every descriptor served by the
  request pool. When disabled, ``AIO_PRIO_DELTA_MAX`` is 0 (``aio_reqprio`` must be 0) and
  ready operations are performed in submission order.
* ``AIO_MAX`` and ``AIO_LISTIO_MAX`` derive from the request pool bounds
  (:kconfig:option:`CONFIG_POSIX_AIO_MAX` and :kconfig:option:`CONFIG_POSIX_AIO_LISTIO_MAX`).

Enable this option with :kconfig:option:`CONFIG_POSIX_ASYNCHRONOUS_IO`.

.. csv-table:: _POSIX_ASYNCHRONOUS_IO
   :header: API, Supported
   :widths: 50,10

    :c:func:`aio_cancel`,yes
    :c:func:`aio_error`,yes
    :c:func:`aio_fsync`,yes
    :c:func:`aio_read`,yes
    :c:func:`aio_return`,yes
    :c:func:`aio_suspend`,yes
    :c:func:`aio_write`,yes
    :c:func:`lio_listio`,yes
