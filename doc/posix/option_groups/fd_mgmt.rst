.. _posix_option_group_fd_mgmt:

POSIX_FD_MGMT
=============

Enable this option group with :kconfig:option:`CONFIG_POSIX_FD_MGMT`.

.. note::
   When using Newlib, Picolibc, or other C libraries conforming to the ISO C Standard, the
   stdio components of the ``POSIX_FD_MGMT`` Option Group are considered supported.

.. csv-table:: POSIX_FD_MGMT
   :header: API, Supported
   :widths: 50,10

    :c:func:`dup`,
    :c:func:`dup2`,
    :c:func:`fcntl`,yes
    :c:func:`fgetpos`,yes
    :c:func:`fseek`,yes
    :c:func:`fseeko`,yes
    :c:func:`fsetpos`,yes
    :c:func:`ftell`,yes
    :c:func:`ftello`,yes
    :c:func:`ftruncate`,yes
    :c:func:`lseek`,yes
    :c:func:`rewind`,yes

.. doxygengroup:: posix_option_group_fd_mgmt
   :project: posix

