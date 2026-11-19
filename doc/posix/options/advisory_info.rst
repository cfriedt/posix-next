.. _posix_option_advisory_info:

_POSIX_ADVISORY_INFO
====================

Enable this option with :kconfig:option:`CONFIG_POSIX_ADVISORY_INFO`.

No cache acts on access-pattern advice, so :c:func:`posix_fadvise` and :c:func:`posix_madvise`
check their arguments and keep the advice for nothing. :c:func:`posix_fallocate` extends the
file to cover the region, which allocates its storage on the file systems Zephyr provides.
:c:func:`posix_memalign` allocates from the heap Zephyr gives every C library.

.. csv-table:: _POSIX_ADVISORY_INFO
   :header: API, Supported
   :widths: 50,10

    :c:func:`posix_fadvise`,yes
    :c:func:`posix_fallocate`,yes
    :c:func:`posix_madvise`,yes
    :c:func:`posix_memalign`,yes

.. doxygengroup:: posix_option_advisory_info
   :project: posix
