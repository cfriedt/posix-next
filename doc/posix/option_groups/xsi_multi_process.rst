.. _posix_option_group_xsi_multi_process:

XSI_MULTI_PROCESS
=================

Enable this option group with :kconfig:option:`CONFIG_XSI_MULTI_PROCESS`.

.. csv-table:: XSI_MULTI_PROCESS
   :header: API, Supported
   :widths: 50,10

    :c:func:`getpriority`,
    :c:func:`getrlimit`,yes
    :c:func:`getrusage`,yes
    :c:func:`nice`,
    :c:func:`setpriority`,
    :c:func:`setrlimit`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`ulimit`,

See :ref:`posix_xsi_multi_process_design` for implementation details.

.. doxygengroup:: posix_option_group_xsi_multi_process
   :project: posix
