.. _posix_option_group_non_portable:

POSIX_NON_PORTABLE
==================

Enable this option group with :kconfig:option:`CONFIG_POSIX_NON_PORTABLE`.

.. csv-table:: POSIX_NON_PORTABLE
   :header: API, Supported
   :widths: 50,10

    :c:func:`pthread_getname_np`,yes
    :c:func:`pthread_setname_np`,yes
    :c:func:`pthread_timedjoin_np`,yes
    :c:func:`pthread_tryjoin_np`,yes

.. doxygengroup:: posix_option_group_non_portable
   :project: posix

