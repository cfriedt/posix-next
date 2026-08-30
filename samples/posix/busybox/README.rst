.. zephyr:code-sample:: posix-busybox
   :name: busybox

   Run busybox applets as processes exec'd from an ext2 file system.

Overview
********

This sample runs `busybox`_, the multi-call binary behind most embedded Linux userlands, on
Zephyr. Busybox is built by the busybox module as a linkable loadable extension (llext) with a
minimal applet set (selected by ``zephyr/busybox.frag`` in the busybox module), installed into an
ext2 file system on a RAM disk as ``/bin/busybox``, and then run with the standard POSIX process
API: each command is a ``posix_spawn()`` of ``/bin/busybox``, which loads the ELF from the file
system, runs its ``main()`` as the process image, and is reaped with ``waitpid()``.

The demo also exercises symbolic links on ext2: it creates one with ``ln -s``, reads it back with
``readlink``, follows it with ``cat``, and finally installs the classic busybox applet link
(``ln -s busybox /bin/ls``) and execs ``/bin/ls`` — exec follows the symbolic link and busybox
dispatches on ``argv[0]``.

Requirements
************

A board with enough RAM for the RAM disk, the llext heap, and the process stacks; the sample is
exercised on ``qemu_riscv64``.

Building and Running
********************

.. zephyr-app-commands::
   :zephyr-app: samples/posix/busybox
   :board: qemu_riscv64
   :goals: run
   :compact:

The demo ends in an interactive hush shell on the console
(:kconfig:option:`CONFIG_ZVFS_STDIN_CONSOLE` wires the reserved stdin descriptor to the
console UART with a minimal line discipline). Builtins, variables, control flow, globbing,
pipelines, and external commands all work: hush runs external pipe members via
posix_spawn(), and every applet is a ``/bin`` symlink installed by ``busybox --install``.
The extension resolves the whole POSIX surface through
:kconfig:option:`CONFIG_POSIX_LLEXT_SYMBOLS`. Leave the shell with ``exit`` or Ctrl-D.

Sample Output
=============

.. code-block:: console

   $ busybox uname -a
   Zephyr zephyr 4.4.1 v4.4.1 ... riscv

   $ busybox echo hello world
   hello world

   $ busybox ln -s motd /etc/motd.lnk

   $ busybox readlink /etc/motd.lnk
   motd

   $ busybox ln -s busybox /bin/ls

   $ ls -l /
   drwxrwxrwx    1         0 tmp
   drwxrwxrwx    1         0 root
   drwxrwxrwx    1         0 etc
   drwxrwxrwx    1         0 bin
   drwxrwxrwx    1         0 lost+found

.. _busybox: https://busybox.net/
