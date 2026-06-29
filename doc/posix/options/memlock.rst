.. _posix_option_memlock:

_POSIX_MEMLOCK
==============

Zephyr's :ref:`Demand Paging API <memory_management_api_demand_paging>` does not yet support
pinning or unpinning all virtual memory regions. The functions below are expected to fail and
set ``errno`` to ``ENOSYS`` :ref:`†<posix_undefined_behaviour>`.

Enable this option with :kconfig:option:`CONFIG_POSIX_MEMLOCK`.

.. csv-table:: _POSIX_MEMLOCK
   :header: API, Supported
   :widths: 50,10

    :c:func:`mlockall`, yes
    :c:func:`munlockall`, yes

.. doxygengroup:: posix_option_memlock
   :project: posix

