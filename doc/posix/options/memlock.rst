.. _posix_option_memlock:

_POSIX_MEMLOCK
==============

Enable this option with :kconfig:option:`CONFIG_POSIX_MEMLOCK`.

Locking keeps pages resident under :kconfig:option:`CONFIG_DEMAND_PAGING`. Without it every
page is resident already and the functions below succeed without effect. See
:ref:`posix_memory_locking_design` for the design.

.. csv-table:: _POSIX_MEMLOCK
   :header: API, Supported
   :widths: 50,10

    :c:func:`mlockall`, yes
    :c:func:`munlockall`, yes

.. doxygengroup:: posix_option_memlock
   :project: posix

