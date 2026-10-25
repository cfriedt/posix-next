.. _posix_option_group_pipe:

POSIX_PIPE
==========

Enable this option group with :kconfig:option:`CONFIG_POSIX_PIPE`.

The number of concurrently open pipes and the pipe buffer capacity (which is
also ``PIPE_BUF``, the atomic write limit) are configured with
:kconfig:option:`CONFIG_ZVFS_PIPE_MAX` and
:kconfig:option:`CONFIG_ZVFS_PIPE_BUF_SIZE`. Linux gives a pipe 64 KiB; a program
written against that capacity (one that lets unread data accumulate) needs the
buffer size raised to match.

A blocking :c:func:`read` or :c:func:`write` on a pipe is interrupted by a signal
the thread will handle: the call fails with ``EINTR``, or returns the bytes already
written, and the handler runs on the way back. This is the one blocking wait signals
interrupt today; see :ref:`posix_signal_interruptible_wait`.

.. csv-table:: POSIX_PIPE
   :header: API, Supported
   :widths: 50,10

    :c:func:`pipe`,yes

.. doxygengroup:: posix_option_group_pipe
   :project: posix

