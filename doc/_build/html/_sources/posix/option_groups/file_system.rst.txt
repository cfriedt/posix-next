.. _posix_option_group_file_system:

POSIX_FILE_SYSTEM
=================

Enable this option group with :kconfig:option:`CONFIG_POSIX_FILE_SYSTEM`.

.. csv-table:: POSIX_FILE_SYSTEM
   :header: API, Supported
   :widths: 50,10

    :c:func:`access`, yes
    :c:func:`chdir`, yes
    :c:func:`closedir`, yes
    :c:func:`creat`, yes
    :c:func:`fchdir`, yes
    :c:func:`fpathconf`, yes
    :c:func:`fstat`, yes
    :c:func:`fstatvfs`, yes
    :c:func:`getcwd`, yes
    :c:func:`link`, yes
    :c:func:`mkdir`, yes
    :c:func:`mkstemp`, yes
    :c:func:`opendir`, yes
    :c:func:`pathconf`, yes
    :c:func:`readdir`, yes
    :c:func:`remove`, yes
    :c:func:`rename`, yes
    :c:func:`rewinddir`, yes
    :c:func:`rmdir`, yes
    :c:func:`stat`, yes
    :c:func:`statvfs`, yes
    :c:func:`tmpfile`, yes
    :c:func:`tmpnam`, yes
    :c:func:`truncate`, yes
    :c:func:`unlink`, yes
    :c:func:`utime`, yes

.. doxygengroup:: posix_option_group_file_system
   :project: posix

