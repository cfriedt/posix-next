.. _posix_option_group_xsi_single_process:

XSI_SINGLE_PROCESS
==================

Enable this option group with :kconfig:option:`CONFIG_XSI_SINGLE_PROCESS`.

.. csv-table:: XSI_SINGLE_PROCESS
   :header: API, Supported
   :widths: 50,10

    :c:func:`gethostid`,yes
    :c:func:`gettimeofday`,yes
    :c:func:`putenv`,yes

.. doxygengroup:: posix_option_group_xsi_single_process
   :project: posix

