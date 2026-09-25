.. zephyr:code-sample:: kcl-toybox
   :name: Linux toybox

   Run toybox, built for Linux with the host's compiler, with an interactive shell.

Overview
********

The sample carries `toybox`_, built as an ordinary static Linux program by
the host's compiler from the toybox module's sources, stripped and compressed
with ``xz`` (the decoder is XZ Embedded, under ``lib/xz``), inflates it at
boot into a root file system that lives in pages mapped as it fills (tmpfs)
with a ``/bin`` link for every one of its applets, and starts its
shell on the console as a process of its own (:c:func:`sys_clone`,
:c:func:`sys_exec_load`, :c:func:`k_waitpid`; no POSIX option is enabled).
Everything the shell does goes
through the Linux kernel compatibility layer
(:kconfig:option:`CONFIG_KCL_LINUX`): its system calls are Linux's, and the
commands it runs are ``fork()`` children that toybox dispatches in-process, so
the sample exercises the layer's process support end to end. Loopback
networking is up, so ``ping`` and ``ping6`` work: their Linux ICMP datagram
sockets are emulated by the layer over the stack's raw ICMP sockets.

The console is a terminal to the programs: ``vi`` edits and saves files, and a
program that a signal ends, ``kill -TERM`` or an ``abort()``, is reported by
the shell with the signal's exit status.

By default the shell is interactive on the console; set
:kconfig:option:`CONFIG_KCL_LINUX_TOYBOX_COMMAND` to run a command line
instead. ``/root/demo.sh`` is installed in the file system: it tours the
applets (pipelines, files, links, FIFOs, a signal-ended child, loopback
pings) and ends in the interactive shell. Twister's default scenario checks
that the interactive prompt comes up; ``sample.kcl.toybox.demo`` runs the
script with ``sh /root/demo.sh`` and matches every step's output in order.

A program that crashes only ends its own process: the sample's fatal error
handler (``src/fatal.c``) closes the descriptors of a user-mode thread that
faulted and lets the kernel abort it, so the shell reports the failure and
goes on. toysh is a pending toybox applet; a ``for`` loop as a pipeline
stage, for one, crashes it on Linux too.

Requirements
************

toybox is built from a pristine export of the toybox module as a static
position-independent Linux executable for the target: with ``cc`` when the
host's architecture is the target's (``qemu_x86_64`` on an x86_64 host), else
with the distribution's Linux cross compiler, ``aarch64-linux-gnu-gcc`` for
``qemu_cortex_a53`` or ``riscv64-linux-gnu-gcc`` for ``qemu_riscv64``, found on
the path. The cross C library must provide static-PIE start files
(``rcrt1.o``): Debian trixie's ``libc6-dev-*-cross`` packages do, Ubuntu
noble's riscv64 one does not. A container can stand in for a missing
toolchain: build an image with the cross packages and put a wrapper named
after the compiler on the path that runs it there with the workspace mounted
on its own paths. A prebuilt static-PIE toybox can be run instead by passing
``-DGUEST=/path/to/toybox``, with ``-DGUEST_APPLETS=/path/to/list`` naming its
applets one per line when it cannot run on the host (``make list`` in a toybox
tree prints them). The build strips the executable with the toolchain's
``strip`` and compresses it with the host's ``xz``.

Building and Running
********************

.. zephyr-app-commands::
   :zephyr-app: samples/subsys/kcl/toybox
   :board: qemu_x86_64
   :goals: run
   :compact:

Type at the ``$`` prompt; ``exit`` ends the shell and the sample. With
``trace.conf`` every Linux system call is logged, for
:file:`scripts/linux_syscall_graph.py`.

Sample Output
=============

.. code-block:: console

   218 applets linked in /bin
   toybox for Linux (2090792 bytes) at /bin/toybox; running sh interactively
   $ uname -a
   Linux zephyr 6.1.0-zephyr #1 Zephyr 4.4.1 x86_64 Linux
   $ cat /etc/motd
   Hello from toybox for Linux on Zephyr!
   $ echo a b c | wc -w
   3
   $ ping -c 1 127.0.0.1
   Ping 127.0.0.1 (127.0.0.1): 56(84) bytes.
   64 bytes from 127.0.0.1: icmp_seq=1 ttl=64 time=10 ms
   --- 127.0.0.1 ping statistics ---
   1 packets transmitted, 1 received, 0% packet loss
   round-trip min/avg/max = 10/10/10 ms
   $ exit
   sh exited with status 0
   kcl linux toybox sample complete

.. _toybox: https://landley.net/toybox/
