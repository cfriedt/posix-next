.. _posix_option_group_job_control:

POSIX_JOB_CONTROL
=================

The ``POSIX_JOB_CONTROL`` Option Group is not yet fully supported in Zephyr. Process
groups are implemented as part of
:ref:`POSIX_MULTI_PROCESS <posix_option_group_multi_process>`, but the terminal-related
interfaces require a controlling terminal, which Zephyr does not have. See also the
:ref:`_POSIX_JOB_CONTROL <posix_option_job_control>` Option.

.. csv-table:: POSIX_JOB_CONTROL
   :header: API, Supported
   :widths: 50,10

    :c:func:`setpgid`,yes
    :c:func:`tcgetpgrp`,yes
    :c:func:`tcgetsid`,yes
    :c:func:`tcsetpgrp`,yes

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_JOB_CONTROL`` Option
Group.

.. doxygengroup:: posix_option_group_job_control
   :project: posix
