.. _posix_option_group_xsi_file_system:

XSI_FILE_SYSTEM
===============

Enable this option group with :kconfig:option:`CONFIG_XSI_FILE_SYSTEM`.

.. csv-table:: XSI_FILE_SYSTEM
   :header: API, Supported
   :widths: 50,10

    :c:func:`basename`,yes
    :c:func:`dirname`,yes
    :c:func:`ftw`,yes
    :c:func:`lockf`,yes
    :c:func:`mknod`,yes
    :c:func:`nftw`,yes
    :c:func:`realpath`,yes
    :c:func:`seekdir`,yes
    :c:func:`sync`,yes
    :c:func:`telldir`,yes
    :c:func:`tempnam`,yes

:c:func:`mknod` makes FIFOs and regular files; no device special file exists to make, so any
other type is refused with ``EINVAL``. :c:func:`sync` writes back every open file description;
data held only by a memory mapping is written back by :c:func:`msync`. A position from
:c:func:`telldir` is the count of entries read since the stream was opened or rewound.
:c:func:`lockf` takes the write locks of :c:func:`fcntl`, which are process-wide: a region the
caller already holds never conflicts, so ``F_TEST`` reports only other processes' locks.

.. doxygengroup:: posix_option_group_xsi_file_system
   :project: posix
