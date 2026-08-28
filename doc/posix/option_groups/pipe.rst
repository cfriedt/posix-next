.. _posix_option_group_pipe:

POSIX_PIPE
==========

Enable this option group with :kconfig:option:`CONFIG_POSIX_PIPE`.

The number of concurrently open pipes and the pipe buffer capacity (which is
also ``PIPE_BUF``, the atomic write limit) are configured with
:kconfig:option:`CONFIG_ZVFS_PIPE_MAX` and
:kconfig:option:`CONFIG_ZVFS_PIPE_BUF_SIZE`.

.. csv-table:: POSIX_PIPE
   :header: API, Supported
   :widths: 50,10

    :c:func:`pipe`,yes

.. doxygengroup:: posix_option_group_pipe
   :project: posix

