.. _posix_option_group_file_system:

POSIX_FILE_SYSTEM
=================

Enable this option group with :kconfig:option:`CONFIG_POSIX_FILE_SYSTEM`.

.. csv-table:: POSIX_FILE_SYSTEM
   :header: API, Supported
   :widths: 50,10

    :c:func:`access`,
    :c:func:`chdir`,
    :c:func:`closedir`, yes
    :c:func:`creat`,
    :c:func:`fchdir`,
    :c:func:`fpathconf`,
    :c:func:`fstat`, yes
    :c:func:`fstatvfs`,
    :c:func:`getcwd`,
    :c:func:`link`,
    :c:func:`mkdir`, yes
    :c:func:`mkstemp`,
    :c:func:`opendir`, yes
    :c:func:`pathconf`,
    :c:func:`readdir`, yes
    :c:func:`remove`, yes
    :c:func:`rename`, yes
    :c:func:`rewinddir`,
    :c:func:`rmdir`, yes
    :c:func:`stat`, yes
    :c:func:`statvfs`,
    :c:func:`tmpfile`,
    :c:func:`tmpnam`,
    :c:func:`truncate`,
    :c:func:`unlink`, yes
    :c:func:`utime`,

.. doxygengroup:: posix_option_group_file_system
   :project: posix

