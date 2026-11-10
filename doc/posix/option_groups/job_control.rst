.. _posix_option_group_job_control:

POSIX_JOB_CONTROL
=================

Enable this Option Group with :kconfig:option:`CONFIG_POSIX_JOB_CONTROL`.

Process groups and sessions come with
:ref:`POSIX_MULTI_PROCESS <posix_option_group_multi_process>`; this group adds the
foreground process group of the console, the one terminal, and the stop and continue
semantics of the job-control signals: the stop signals suspend a process, ``SIGCONT``
resumes it, and :c:func:`waitpid` reports the transitions with ``WUNTRACED`` and
``WCONTINUED``. See also the :ref:`_POSIX_JOB_CONTROL <posix_option_job_control>` Option.

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
