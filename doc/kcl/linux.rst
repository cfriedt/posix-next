.. _kcl_linux:

Linux
#####

The Linux layer (:kconfig:option:`CONFIG_KCL_LINUX`) makes every Linux system
call of the architecture a system call of the Zephyr image, at its Linux
number. Zephyr already invokes system calls the way Linux does on x86_64
(``syscall``), AArch64 (``svc``) and RISC-V (``ecall``), so a user-mode thread
executing Linux binary code has its system calls dispatched to the layer
without any change to the trap path. Calls the layer does not provide fail
with ``ENOSYS``.

Architectures
*************

A Linux program owns the thread-pointer register in user mode (``fs`` base,
``tpidr_el0``, ``tp``) while the kernel keeps its own TLS base per thread:
:kconfig:option:`CONFIG_USER_THREAD_POINTER` switches the register on every
transition between user and kernel mode. Where Linux's system call convention
differs from Zephyr's, :kconfig:option:`CONFIG_USER_SYSCALL_ABI` lets the
layer switch a thread to it as it enters the executable: on RISC-V the number
is in ``a7`` rather than ``t0`` (and ``gp`` is the program's too), on ARM64 the
trap is ``svc #0``. On x86_64 the two conventions coincide. The forked child
of a Linux process inherits both settings.

System calls
************

The layer's system calls are named ``zkcl_linux_syscall_<name>()`` after the
Linux system call, with the prototype of the Linux kernel's implementation:
Linux types become ``zkcl_linux_*`` types of
:file:`include/zephyr/kcl/linux/types.h`. Their numbers are pinned with
``zephyr_syscall_reserve()``: :file:`scripts/build/gen_syscalls.py` gives the
listed calls their Linux numbers and packs Zephyr's own system calls into the
IDs left over.

:file:`subsys/kcl/linux/syscalls.json` holds every call's prototype and its
number on each architecture, rebuilt by
:file:`subsys/kcl/linux/gen_linux_syscalls.py` from the
`system-calls <https://github.com/hrw/syscalls-table>`_ package and a Linux
source tree; the build generates the architecture's declarations, number table
and verifiers from it at configure time.
Implementations, ``z_impl_zkcl_linux_syscall_<name>()``, call the virtual file
system, the process layer and the kernel, validate the executable's pointers
themselves, and return the C library's errno numbers, which the verifier
renumbers to Linux's.

Every call the layer implements has an option of its own,
``CONFIG_KCL_LINUX_SYSCALL_<NAME>`` (:file:`subsys/kcl/linux/Kconfig.syscalls`),
on by default and depending on the Zephyr service the implementation is built
on: :kconfig:option:`CONFIG_KCL_LINUX` *implies* those services rather than
selecting them, so all are on by default and any one, or any single call, can
be switched off, which drops its implementation from the image; the call then
fails with ENOSYS like one the layer never had.

Executables
***********

With :kconfig:option:`CONFIG_KCL_LINUX_EXEC`, :c:func:`sys_exec_load` loads
static Linux executables: position-independent (``-static-pie``) ELF images
without an interpreter. Their segments are copied into memory owned by the
process and mapped into its domain with the segment's access, followed by an
area for ``brk()`` and a stack. The leader builds the initial stack as Linux
does, with the argument, environment and auxiliary vectors, and enters the
executable in user mode. Any process creation over :c:func:`sys_exec_load`,
:c:func:`posix_spawn` and :c:func:`execve` included, therefore takes Linux
executables like any other image. The layer depends on no POSIX option: it
selects the kernel, ``sys_`` and ZVFS services it is built on.

A Linux executable runs its own thread-local storage: on x86_64 the FS base it
sets with ``arch_prctl()`` is switched in on every return to user mode and the
kernel's own TLS base on every entry (:kconfig:option:`CONFIG_USER_THREAD_POINTER`).

Tracing
*******

The generated verifiers wrap every implementation in the tracing subsystem's
system call hooks, so :kconfig:option:`CONFIG_TRACING_SYSCALL` records a Linux
process's calls like Zephyr's own. With :kconfig:option:`CONFIG_TRACING_USER`
an application implements ``sys_trace_syscall_enter_user()`` and
``sys_trace_syscall_exit_user()``; the kcl/hello sample logs them and
:file:`scripts/linux_syscall_graph.py` draws the sequence as a graph.

Limitations
***********

* Fixed-address (``ET_EXEC``) and dynamically linked executables are refused.
* Sockets are the network stack's; the address families and address
  structures are converted, and a Linux ICMP datagram ("ping") socket is
  emulated on a raw ICMP socket.
* ``mmap()`` provides anonymous private memory and copies of files: a file
  mapping is read into memory when it is made, and a shared writable one is
  written back through its descriptor at ``munmap()`` and ``msync()`` while
  that descriptor still names the file; ``mprotect()`` is accepted without
  effect.
* ``fork()``, ``vfork()`` and the fork flavour of ``clone()`` create a child
  with a copy of the address space (:kconfig:option:`CONFIG_PROCESS_VM`);
  threads and ``execve()`` from a Linux process are not provided yet, and the
  child's tid pointers of ``clone()`` are not written.
* ``mknod()`` creates FIFOs and regular files; there are no device or socket
  nodes to create, so those requests fail with ``EPERM``.
* Signals 1 to 31 share the kernel's numbers: the signal mask and the default
  and ignore dispositions are the kernel's, so ``kill()`` ends a process with a
  wait status that names the signal, but a handler installed by the process is
  never called and a signal it handles is dropped. ``futex()`` provides the
  wait and wake operations of a single-threaded process.
* The console is the terminal: the canonical, echo and CR-to-NL input modes of
  ``termios`` reach it and ``poll()`` waits on it, so full-screen programs run;
  its size is reported as 80 by 24.
* x86_64, aarch64 and riscv64 run Linux executables; the generated tables
  also exist for riscv32, arm and i386, whose entry paths are not done.

API Reference
*************

.. doxygengroup:: kcl_linux
