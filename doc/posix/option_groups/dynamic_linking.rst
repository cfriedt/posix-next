.. _posix_option_group_dynamic_linking:

POSIX_DYNAMIC_LINKING
=====================

The ``POSIX_DYNAMIC_LINKING`` Option Group is not yet supported in Zephyr. Zephyr provides similar
functionality for loading and linking extensions at runtime via :ref:`llext`.

.. csv-table:: POSIX_DYNAMIC_LINKING
   :header: API, Supported
   :widths: 50,10

    :c:func:`dlclose`,no
    :c:func:`dlerror`,no
    :c:func:`dlopen`,no
    :c:func:`dlsym`,no

Please refer to `Subprofiling Considerations <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_ for details on the ``POSIX_DYNAMIC_LINKING`` Option
Group.
