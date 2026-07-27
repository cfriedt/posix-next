.. _posix_option_memlock:

_POSIX_MEMLOCK
==============

Without :kconfig:option:`CONFIG_DEMAND_PAGING`, all memory is always resident and the
functions below succeed trivially. With demand paging enabled, Zephyr's
:ref:`Demand Paging API <memory_management_api_demand_paging>` does not yet support pinning
or unpinning all virtual memory regions, so ``mlockall()`` fails with ``EAGAIN``.

Enable this option with :kconfig:option:`CONFIG_POSIX_MEMLOCK`.

.. csv-table:: _POSIX_MEMLOCK
   :header: API, Supported
   :widths: 50,10

    :c:func:`mlockall`, yes
    :c:func:`munlockall`, yes

.. doxygengroup:: posix_option_memlock
   :project: posix

