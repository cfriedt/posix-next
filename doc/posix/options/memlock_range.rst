.. _posix_option_memlock_range:

_POSIX_MEMLOCK_RANGE
====================

Enable this option with :kconfig:option:`CONFIG_POSIX_MEMLOCK_RANGE`.

.. csv-table:: _POSIX_MEMLOCK_RANGE
   :header: API, Supported
   :widths: 50,10

    :c:func:`mlock`, yes
    :c:func:`munlock`, yes

.. doxygengroup:: posix_option_memlock_range
   :project: posix

