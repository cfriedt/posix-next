.. zephyr:code-sample:: posix-toybox
   :name: toybox

   Run toybox commands as processes exec'd from an ext2 file system.

Overview
********

This sample runs `toybox`_, a multi-call binary providing the standard command line utilities, on
Zephyr. Toybox is built by the toybox module as a linkable loadable extension (llext) with its
default command set, less the commands that need Linux kernel interfaces Zephyr does not have
(``zephyr/toybox.frag`` in the toybox module lists them), installed into an ext2 file system on a
RAM disk as ``/bin/toybox`` with a ``/bin`` symlink for every command, and then run with the
standard POSIX process API: :c:func:`posix_spawn` and :c:func:`waitpid`. Toybox dispatches on
the name it was invoked by, so each command is available under its own name.

Networking is loopback-only by default. The ``gateway`` scenario (``qemu_cortex_a53``) puts the
image on qemu's user-mode network instead: it pings the virtual gateway over IPv4 and IPv6 and
resolves a name through Zephyr's DNS resolver, configured with Google's public servers, so
``ping``, ``wget`` and ``nc`` work by name from the shell when the host has internet access.

Building and Running
********************

.. zephyr-app-commands::
   :zephyr-app: samples/posix/toybox
   :board: qemu_riscv64
   :goals: run
   :compact:

.. _toybox: https://landley.net/toybox/
