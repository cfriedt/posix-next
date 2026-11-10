.. zephyr:code-sample:: kcl-hello
   :name: Linux hello

   Spawn a Linux program, built with the host's compiler, as a process.

Overview
********

The sample carries a static, position-independent Linux executable as an
``.xz`` stream, inflates it at boot into a root file system that lives in
pages mapped as it fills (tmpfs), and runs it as a process of its own, created
with :c:func:`sys_clone`, loaded with :c:func:`sys_exec_load` and reaped with
:c:func:`k_waitpid`, exactly as it would run a Zephyr image. Nothing of the
POSIX API is involved: the layer selects the kernel, ``sys_`` and ZVFS
services it is built on. The program is an ordinary C program (``guest/hello.c``)
compiled for the host with the host's C library: it prints a greeting, its
arguments and environment, and what ``uname()`` reports, then exits with
status 42.

The executable's system calls are the architecture's Linux system calls,
served by the Linux kernel compatibility layer
(:kconfig:option:`CONFIG_KCL_LINUX`) from the virtual file system, the
process layer and the kernel.

Requirements
************

The guest is built as a static position-independent Linux executable for the
target: with ``cc`` when the host's architecture is the target's
(``qemu_x86_64`` on an x86_64 host), else with the distribution's Linux cross
compiler, ``aarch64-linux-gnu-gcc`` for ``qemu_cortex_a53`` or
``riscv64-linux-gnu-gcc`` for ``qemu_riscv64``, whose C library must provide
static-PIE start files (``rcrt1.o``; Debian trixie's cross packages do). Any
other static-PIE Linux executable can be run instead by passing
``-DGUEST=/path/to/executable``.

Building and Running
********************

.. zephyr-app-commands::
   :zephyr-app: samples/subsys/kcl/hello
   :board: qemu_x86_64
   :goals: run
   :compact:

Tracing the system calls
========================

With ``trace.conf`` the user tracing backend logs every Linux system call the
guest makes (``src/trace.c`` implements its system call hooks), and
:file:`scripts/linux_syscall_graph.py` draws the sequence as a graph:

.. code-block:: console

   west build -b qemu_x86_64 samples/subsys/kcl/hello -- -DEXTRA_CONF_FILE=trace.conf
   west build -t run | tee hello.log
   scripts/linux_syscall_graph.py hello.log --svg hello.svg

Each node is a system call with the number of times it was made, each edge a
transition to the next call with its count.

Sample Output
=============

.. code-block:: console

   running /bin/hello (822776 bytes)
   hello, world
   pid 8, 3 argument(s): /bin/hello from zephyr
   env: HOME=/
   env: LANG=C
   running on Linux 6.1.0-zephyr x86_64
   /bin/hello exited with status 42
   kcl linux hello sample complete
