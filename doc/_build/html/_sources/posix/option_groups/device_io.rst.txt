.. _posix_option_group_device_io:

POSIX_DEVICE_IO
===============

Enable this option group with :kconfig:option:`CONFIG_POSIX_DEVICE_IO`.

.. note::
   When using Newlib, Picolibc, or other C libraries conforming to the ISO C Standard, the
   C89 components of the ``POSIX_DEVICE_IO`` Option Group are considered supported.

.. csv-table:: POSIX_DEVICE_IO
   :header: API, Supported
   :widths: 50,10

    :c:func:`FD_CLR`,yes
    :c:func:`FD_ISSET`,yes
    :c:func:`FD_SET`,yes
    :c:func:`FD_ZERO`,yes
    :c:func:`clearerr`,yes
    :c:func:`close`,yes
    :c:func:`fclose`,yes
    :c:func:`fdopen`,yes
    :c:func:`feof`,yes
    :c:func:`ferror`,yes
    :c:func:`fflush`,yes
    :c:func:`fgetc`,yes
    :c:func:`fgets`,yes
    :c:func:`fileno`,yes
    :c:func:`fopen`,yes
    :c:func:`fprintf`,yes
    :c:func:`fputc`,yes
    :c:func:`fputs`,yes
    :c:func:`fread`,yes
    :c:func:`freopen`,yes
    :c:func:`fscanf`,yes
    :c:func:`fwrite`,yes
    :c:func:`getc`,yes
    :c:func:`getchar`,yes
    :c:func:`gets`,yes
    :c:func:`open`,yes
    :c:func:`perror`,yes
    :c:func:`poll`,yes
    :c:func:`printf`,yes
    :c:func:`pread`,yes
    :c:func:`pselect`,yes
    :c:func:`putc`,yes
    :c:func:`putchar`,yes
    :c:func:`puts`,yes
    :c:func:`pwrite`,yes
    :c:func:`read`,yes
    :c:func:`scanf`,yes
    :c:func:`select`,yes
    :c:func:`setbuf`,yes
    :c:func:`setvbuf`,yes
    stderr,yes
    stdin,yes
    stdout,yes
    :c:func:`ungetc`,yes
    :c:func:`vfprintf`,yes
    :c:func:`vfscanf`,yes
    :c:func:`vprintf`,yes
    :c:func:`vscanf`,yes
    :c:func:`write`,yes

.. doxygengroup:: posix_option_group_device_io
   :project: posix

