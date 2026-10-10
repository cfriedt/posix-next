.. _posix_option_memlock_range:

_POSIX_MEMLOCK_RANGE
====================

Enable this option with :kconfig:option:`CONFIG_POSIX_MEMLOCK_RANGE`.

Locking keeps pages resident under :kconfig:option:`CONFIG_DEMAND_PAGING`. Without it every
page is resident already and the functions below succeed without effect. See
:ref:`posix_memory_locking_design` for the design.

.. csv-table:: _POSIX_MEMLOCK_RANGE
   :header: API, Supported
   :widths: 50,10

    :c:func:`mlock`, yes
    :c:func:`munlock`, yes

.. doxygengroup:: posix_option_memlock_range
   :project: posix

