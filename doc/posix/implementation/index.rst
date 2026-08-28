.. _posix_details:

Implementation Details
######################

In many ways, Zephyr provides support like any POSIX OS; API bindings are provided in the C
programming language, POSIX headers are available in the standard include path, when configured.

Unlike other multi-purpose POSIX operating systems

- Zephyr is not "a POSIX OS". The Zephyr kernel was not designed around the POSIX standard, and
  POSIX support is an opt-in feature
- Zephyr, libraries, and application code are compiled and linked together into one artifact,
  running by default like a single-process application, in a single (possibly virtual) address
  space
- Processes are opt-in: with :ref:`POSIX_MULTI_PROCESS <posix_option_group_multi_process>`, a
  process is a kernel thread group, additional process images are prelinked into the same
  artifact or loaded from the file system as ELF extensions, and :c:func:`fork` duplicates an
  address space where the hardware supports it (see :ref:`posix_multi_process_design`)
- Zephyr does not provide a POSIX shell, compiler, utilities, and is not self-hosting.

.. note::
   Unlike the Linux kernel or FreeBSD, Zephyr does not maintain a static table of system call
   numbers for each supported architecture, but instead generates system calls dynamically at
   build time. See :ref:`System Calls <syscalls>` for more information.

Design
======

As a library, Zephyr's POSIX API implementation makes an effort to be a thin abstraction layer
between the application, middleware, and the Zephyr kernel.

Some general design considerations:

- The POSIX interface and implementations should be part of Zephyr's POSIX library, and not
  elsewhere, unless required both by the POSIX API implementation and some other feature. An
  example where the implementation should remain part of the POSIX implementation is
  ``getopt()``. Examples where the implementation should be part of separate libraries are
  multithreading and networking.

- When the POSIX API and another Zephyr subsystem both rely on a feature, the implementation of
  that feature should be as a separate Zephyr library that can be used by both the POSIX API and
  the other library or subsystem. This reduces the likelihood of dependency cycles in code. When
  practical, that rule should expand to include macros. In the example below, ``libposix``
  depends on ``libzfoo`` for the implementation of some functionality "foo" in Zephyr. If
  ``libzfoo`` also depends on ``libposix``, then there is a dependency cycle. The cycle can be
  removed via mutual dependency, ``libcommon``.

.. graphviz::
   :caption: Dependency cycle between POSIX and another Zephyr library

   digraph {
       node [shape=rect, style=rounded];
       rankdir=LR;

       libposix [fillcolor="#d5e8d4"];
       libzfoo [fillcolor="#dae8fc"];

       libposix -> libzfoo;
       libzfoo -> libposix;
   }

.. graphviz::
   :caption: Mutual dependencies between POSIX and other Zephyr libraries

   digraph {
       node [shape=rect, style=rounded];
       rankdir=LR;

       libposix [fillcolor="#d5e8d4"];
       libzfoo [fillcolor="#dae8fc"];
       libcommon [fillcolor="#f8cecc"];

       libposix -> libzfoo;
       libposix -> libcommon;
       libzfoo -> libcommon;
   }

- POSIX API calls should be provided as regular callable C functions; if a Zephyr
  :ref:`System Call <syscalls>` is needed as part of the implementation, the declaration and the
  implementation of that system call should be hidden behind the POSIX API.

Organization and Source Layout of POSIX Options and Option Groups
=================================================================

IEEE Std 1003.1 defines POSIX
`Options <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap02.html#tag_02_01_03>`_
and
`Subprofiling Option Groups <https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html>`_
separately. Zephyr maps each standard Option Group to a directory under ``lib/posix/``.

When an implementation supports an Option Group (or an Option), it is required to define a constant
to indicate support for that Option Group (or Option) for C source files. For example, if the
implementation supports the ``POSIX_TIMERS`` Option Group, it is required to define the macro
``_POSIX_TIMERS`` to a specific value. In most cases, the Option Group names and the associated Option
(and macro) names differ by only one character (the prefixing underscore).

However, some Options and Option Groups are intentionally not published as Subprofiling Option
Groups because they do not meet the criteria established by
`IEEE Std 1003.13 <https://standards.ieee.org/ieee/1003.13/3322/>`_. Namely, that Subprofiling Option
Groups must

- have a minimal footprint
  - to facilitate specialized, embedded, resource-constrained target devices, and
- independence
  - to decouple functionality from other Options and Subprofiling Option Groups.

See also :ref:`posix_aep` for how Zephyr maps PSE51/52/53 choices onto individual Kconfig options.

For example, the :ref:`XSI_REALTIME <posix_option_group_xsi_realtime>` Option Group and
the :ref:`_POSIX_MESSAGE_PASSING <posix_option_message_passing>` depend on other Options or Option
Groups (:ref:`POSIX_DEVICE_IO <posix_option_group_device_io>`,
:ref:`POSIX_REALTIME_SIGNALS <posix_option_group_realtime_signals>`, etc) and therefore are not
qualified to be standard Subprofiling Option Groups.

For simplicity and maintainability, Zephyr organizes such Options and Option Groups the same as
standard Subprofiling Option Groups (at the discretion of the maintainer), under
``lib/posix/``.

The general rule is that Option Groups will *always* have an associated Kconfig option and *some*
Options (but not all) have an associated Kconfig option in Zephyr. The latter is mostly for
maintainability.

Native POSIX Thread Library (NPTL)
==================================

Zephyr's POSIX threading implementation follows the same design philosophy as the
`Native POSIX Thread Library (NPTL) <https://www.akkadia.org/drepper/nptl-design.pdf>`_
found in glibc on Linux: every POSIX primitive maps 1:1 to a native kernel object. There is no
user-space scheduler or M:N multiplexing layer — each ``pthread_t`` *is* a ``k_thread``, each
``pthread_mutex_t`` *is* a ``k_mutex``, and each ``pthread_cond_t`` *is* a ``k_condvar``.

.. graphviz::
   :caption: 1:1 mapping between POSIX and Zephyr kernel objects

   digraph {
       rankdir=LR;
       node [shape=record, style=filled];

       subgraph cluster_posix {
           label="POSIX API";
           style=filled;
           color="#e8f5e9";
           fillcolor="#e8f5e9";
           pt  [label="pthread_t"        fillcolor="#c8e6c9"];
           pm  [label="pthread_mutex_t"  fillcolor="#c8e6c9"];
           pc  [label="pthread_cond_t"   fillcolor="#c8e6c9"];
       }

       subgraph cluster_kernel {
           label="Zephyr Kernel";
           style=filled;
           color="#e3f2fd";
           fillcolor="#e3f2fd";
           kt  [label="k_thread"   fillcolor="#bbdefb"];
           km  [label="k_mutex"    fillcolor="#bbdefb"];
           kc  [label="k_condvar"  fillcolor="#bbdefb"];
       }

       pt -> kt [label="1:1" style=bold];
       pm -> km [label="1:1" style=bold];
       pc -> kc [label="1:1" style=bold];
   }

The POSIX types (``pthread_t``, ``pthread_mutex_t``, ``pthread_cond_t``) are opaque integer handles
whose value is derived from the address of the underlying kernel object in a system-wide pool. The
conversion is performed by ``to_k_thread()``, ``to_k_mutex()``, and ``to_k_condvar()`` (and their
inverses) defined in the internal header ``posix_internal.h``.

This 1:1 design means:

- No extra scheduling layer — every POSIX thread is a kernel thread (and vice versa)
- Kernel-level visibility — debuggers and trace tools see the same objects as the application.
- **Full userspace support** — because every POSIX call bottoms out in a Zephyr system call
  operating on a kernel object, the entire POSIX API is available to both privileged and
  unprivileged (userspace) threads. As with everything userspace, it is important to keep in mind
  that user threads do not have permission on any kernel objects by default.
- **POSIX is optional** - POSIX is entirely optional in Zephyr. However, in order to use POSIX
  features, it is highly recommended to enable one of the
  :ref:`POSIX subprofiles <posix_aep>` such as ``CONFIG_POSIX_AEP_CHOICE_PSE51``.


.. _posix_scheduling_priorities:

Scheduling Priorities
=====================

POSIX and Zephyr order their scheduling priorities in opposite directions. POSIX priorities
are non-negative and numerically *larger is more urgent*: each policy advertises its range
through :c:func:`sched_get_priority_min` and :c:func:`sched_get_priority_max`. Zephyr's
native priorities are numerically *smaller is more urgent*: cooperative threads occupy the
negative values and preemptible threads the non-negative ones.

.. graphviz::
   :caption: Zephyr's native priority axis. POSIX priorities map onto it reversed, so the
             POSIX maximum for a policy corresponds to the most negative (leftmost) Zephyr
             value in that policy's band.

   digraph zephyr_priority_axis {
       rankdir=LR;
       node [shape=box, style=rounded, fontname="sans-serif", fontsize=10];
       edge [fontname="sans-serif", fontsize=9, arrowsize=0.6];

       urgent [label="most urgent\n(POSIX max)", shape=plaintext];
       relaxed [label="least urgent\n(POSIX min)", shape=plaintext];

       subgraph cluster_coop {
           label="cooperative (SCHED_FIFO)";
           fontname="sans-serif";
           fontsize=10;
           style=dashed;
           cm [label="-CONFIG_NUM_COOP_PRIORITIES"];
           cd [label="…", shape=plaintext];
           c1 [label="-1"];
       }

       subgraph cluster_preempt {
           label="preemptible (SCHED_RR, SCHED_OTHER)";
           fontname="sans-serif";
           fontsize=10;
           style=dashed;
           p0 [label="0"];
           pd [label="…", shape=plaintext];
           pn [label="CONFIG_NUM_PREEMPT_PRIORITIES - 1"];
       }

       urgent -> cm [dir=none];
       cm -> cd -> c1 [dir=none];
       c1 -> p0 [dir=none];
       p0 -> pd -> pn [dir=none];
       pn -> relaxed [dir=none];
   }

The POSIX layer converts exactly once, at its boundary: :c:func:`pthread_setschedparam`,
:c:func:`sched_setscheduler`, and friends map a POSIX priority into the Zephyr band for the
requested policy, and everything below the boundary works exclusively in Zephyr's native
space.

This is why prioritized I/O orders its ready queue with a *min*-heap even though POSIX
phrases the ordering as a maximum. ``aio_reqprio`` is a non-negative delta that can only
*lower* a request below its submitting thread - by itself it orders nothing, being relative
to the submitter - so the effective POSIX priority is the thread's priority *minus* the
delta, and selecting the next request would take the numeric *maximum* of those values.
Expressed in Zephyr's native space the same subtraction becomes
``k_thread_priority_get() + aio_reqprio`` - one Zephyr level per unit, moving *right* along
the axis above - and the most urgent request is now the numeric *minimum*. ``sys_aio`` keys
its ready heap on that native value paired with a wraparound-safe submission sequence
number, so equal-priority requests complete first-in-first-out, as prioritized I/O
requires.

.. _posix_sporadic_server:

Sporadic Server Scheduling
==========================

The Sporadic Server options (``_POSIX_SPORADIC_SERVER`` and ``_POSIX_THREAD_SPORADIC_SERVER``)
describe the ``SCHED_SPORADIC`` scheduling policy, under which a thread runs at a foreground
priority while it has execution budget remaining, is demoted to a background priority
(``sched_ss_low_priority``) when the budget is exhausted, and has its budget restored through a
queue of pending replenishment operations scheduled one replenishment period after each
activation.

With the Thread Sporadic Server option
(:kconfig:option:`CONFIG_POSIX_THREAD_SPORADIC_SERVER`), Zephyr accepts ``SCHED_SPORADIC`` and
validates the sporadic server scheduling parameters, but does not enforce execution-time
budgets: a thread scheduled under ``SCHED_SPORADIC`` executes as if scheduled under
``SCHED_RR`` at ``sched_priority``. This deviation is denoted with the
:ref:`† (obelus) <posix_undefined_behaviour>` wherever the option is listed. The process-level
option, ``_POSIX_SPORADIC_SERVER``, is reported as unsupported (``-1``).

The decision not to implement the sporadic server algorithm itself is deliberate:

**Known specification defects.**
   The replenishment algorithm as specified in IEEE Std 1003.1 contains well-documented defects.
   Under certain preemption and blocking patterns, a literal implementation of the specified
   rules produces premature replenishments, allowing a thread scheduled under ``SCHED_SPORADIC``
   to consume substantially more processor time than its nominal budget — up to the entire
   processor in the worst case — thereby defeating the temporal isolation the policy is intended
   to provide. See M. Stanovich, T. P. Baker, A. Wang, and M. González Harbour, *Defects of the
   POSIX Sporadic Server and How to Correct Them*, in Proceedings of the 16th IEEE Real-Time and
   Embedded Technology and Applications Symposium (RTAS), 2010. Corrected variants exist in the
   literature, but they deviate from the standardized algorithm; an implementation must choose
   between fidelity to the specification and correct budget enforcement.

**Cost imposed on the scheduler hot path.**
   Budget enforcement requires the kernel to maintain per-thread execution-time accounting,
   per-thread queues of pending replenishment operations (bounded by ``sched_ss_max_repl``,
   with merging logic when replenishments coalesce), and automatic priority switching on budget
   exhaustion and replenishment. Every context switch, preemption, and blocking operation
   touches this state. That bookkeeping conflicts with Zephyr's goals of small, deterministic,
   low-overhead kernel primitives and would tax all users of the scheduler, including those who
   do not use the policy.

**Limited adoption.**
   The algorithm is rarely implemented. Linux, FreeBSD, and most embedded and general-purpose
   operating systems omit ``SCHED_SPORADIC`` entirely; conforming applications must already
   handle its absence or partial support via the standard feature-test mechanisms.

**Mature alternatives.**
   Applications requiring bounded execution or temporal isolation are better served by
   mechanisms that are already available and better understood:

   - :kconfig:option:`CONFIG_SCHED_DEADLINE` provides earliest-deadline-first scheduling,
     the same family of algorithms adopted by Linux in preference to the sporadic server model.
   - :ref:`_POSIX_THREAD_CPUTIME <posix_option_thread_cputime>` (``CLOCK_THREAD_CPUTIME_ID``)
     permits per-thread execution-time measurement, and together with
     :ref:`_POSIX_TIMERS <posix_option_timers>` allows an application to implement
     budget-monitoring policies in user space, with policy decisions (demotion, throttling,
     logging) tailored to the application rather than fixed by the kernel.

.. _posix_multi_process_design:

Multi-Process Design
====================

This section documents the kernel and system-layer substrate that backs the
:c:func:`fork`, :c:func:`execve`, and ``posix_spawn`` families in the
``POSIX_MULTI_PROCESS`` and ``POSIX_SPAWN`` Option Groups.

Process model
-------------

A process is a **thread group**: its identity is its thread-group leader, so a
process handle (``k_pid_t``) is a thread handle. ``struct k_process`` is the
kernel-owned record of one thread group and hangs off the leader; it is not a
handle and, except when a caller-provided record crosses the user/kernel
boundary, not a kernel object. Foreign handles are resolved by identity-value
scans under a lock and never dereferenced, so a handle stays meaningful from
creation to reap - including after the leader thread has exited.

Numeric process, group, and session IDs are a *system-layer* concept, not a
kernel one: the kernel speaks only in handles, and ``lib/os`` maintains the
numbering tables behind :c:func:`sys_process_id` and friends. All allocation is
likewise confined to ``lib/os``: the kernel never allocates and never calls an
allocator, only initializing caller-provided or statically defined records.

.. graphviz::

   digraph process_layers {
       rankdir=LR;
       node [shape=box, fontname="Helvetica"];
       app [label="POSIX layer\nfork / execve / posix_spawn\n(numeric pid_t)"];
       sys [label="system layer (lib/os)\nsys_clone / sys_setsid\nnumbering, pools"];
       kern [label="kernel\nk_process_init / k_kill / k_waitpid\n(handles only, no allocation)"];
       app -> sys -> kern;
       app -> kern [label="handle APIs", style=dotted];
   }

Creation: sys_clone
-------------------

``sys_clone()`` is the creation primitive; :c:func:`fork`, ``posix_spawn``,
:c:func:`pthread_create`, and ``sys_thread_create()`` are library constructs
over it. With no flags it produces a fresh process from a
pooled record and a pool-drawn leader thread. ``SYS_CLONE_THREAD`` selects
the thread tier: the new thread joins the caller's thread group instead -
and without :kconfig:option:`CONFIG_PROCESS` it is simply a new thread, so
thread creation behaves identically in process-less builds.
``SYS_CLONE_PAUSED`` leaves the created thread stopped so a caller can apply
attributes (process group, signal mask, scheduling) before starting it.
``SYS_CLONE_VM_COPY`` selects the fork model. Beneath the system layer,
``k_clone()`` is the kernel primitive with the same two tiers over entirely
caller-provided memory - record, thread, and stack - allocating nothing.

.. graphviz::

   digraph clone {
       node [shape=box, fontname="Helvetica"];
       args [label="sys_clone(args)"];
       vm   [label="SYS_CLONE_VM_COPY?"; shape=diamond];
       fork [label="z_sys_clone_vm_copy\n(duplicate address space,\nkernel-assisted resume)"];
       thr  [label="SYS_CLONE_THREAD?"; shape=diamond];
       member[label="thread in the\ncalling process"];
       fresh[label="fresh leader\n(pool thread + entry)"];
       init [label="k_process_init\n(adopt leader, register)"];
       start[label="SYS_CLONE_PAUSED?\nstart or hand back stopped"; shape=diamond];
       args -> vm;
       vm -> fork [label="yes"];
       vm -> thr [label="no"];
       thr -> member [label="yes"];
       thr -> fresh [label="no"];
       member -> start;
       fresh -> init -> start;
       fork -> init;
   }

The fork model and its substrate
--------------------------------

The fork model duplicates the caller's writable memory into the child at
identical virtual addresses and resumes the child at the call site. Copied
memory at identical addresses is what makes the model sound where ``vfork``
was not: the child cannot disturb the parent's stack, and a plain
``setjmp``/``longjmp`` resumes the child because every address it restores is
valid in its own copy - no architecture context-copy code is required.

The substrate is experimental (:kconfig:option:`CONFIG_PROCESS_VM`, x86 and aarch64).
``process_vm_clone()`` builds the child's memory domain by mirroring the
parent's partitions, then repoints the writable ranges and the stack at fresh
frames through the architecture hook ``z_mem_domain_clone_remap()`` (no TLB
shootdown, since the child is not yet live on any CPU).

The child's resume is **kernel-assisted** (``ARCH_HAS_FORK_RESUME``; x86-64,
x86, and aarch64):
the syscall entry captures the caller's complete register state - the
callee-saved registers are saved alongside the frame precisely because the C
convention would otherwise leave a forked child with no history to restore
them from - and the child's first run enters the common syscall exit path
from a staged copy of that frame with the return-value register zeroed. The
child's TLS is the parent's address: the clone already copied that region,
so captured TLS-variable addresses and register-relative accesses agree,
as fork demands. This is possible because ``CONFIG_PROCESS_VM`` excludes
:kconfig:option:`CONFIG_CURRENT_THREAD_USE_TLS`, leaving kernel mode with
no reason to dereference user TLS. fork() is therefore a user-mode
operation: a kernel-mode caller has no syscall frame to resume and receives
``ENOTSUP``; kernel-mode callers create processes with :c:func:`posix_spawn`
or ``sys_clone()``, which require no duplication of the caller.

.. note::
   **Confirmed working (x86-64, x86, and aarch64):** fork() returns twice through
   the kernel resume, the child diverges on its own stack and data copies,
   and the parent - whose view is asserted unchanged - reaps it. On aarch64
   no dedicated resume path was needed at all: every aarch64 thread is born
   through the exception return path popping a synthetic ESF, so the forked
   child's birth context simply *is* the parent's captured syscall frame
   with the return-value register zeroed.

Per-process permissions
-----------------------

With :kconfig:option:`CONFIG_PROCESS` and userspace, the process - not the
thread - is the kernel-object permission principal: threads sharing an address
space can already reach each other's memory, so per-thread object permissions
within a process add no isolation. Each object's permission bitmap is sized by
:kconfig:option:`CONFIG_MAX_PROCESS_BYTES` rather than
``CONFIG_MAX_THREAD_BYTES``, the principal index lives once in
``struct k_process``, and one sweep at reap replaces the per-thread-death walk.
Granting to any member thread grants the whole process, and a forking child
inherits the parent's grants.

Signals
-------

Signals are delivered either to a specific thread (:c:func:`pthread_kill`) or to
a process (:c:func:`kill`, :c:func:`sigqueue`, and the ``k_kill_all()``
broadcast behind ``kill(-1, ...)``). A process-directed signal is delivered to
one member thread that has it unblocked, preferring the caller for a
self-signal. When every member has the signal masked it pends at **process
scope** and is claimed by whichever member first unblocks or waits for it -
strict POSIX semantics for fully-masked processes. Kernel-resolved process
delivery bypasses the per-thread-object permission check, since :c:func:`kill`
authorization is process-level.

Signal dispositions are likewise a property of the process: installed by any
member thread, in force for every member, and purged when the process is
reaped. ``sys_clone()`` duplicates the parent's dispositions into a new
process - every disposition for the fork tier, only ignored dispositions for a
fresh image (fork+exec semantics) - and :c:func:`execve` reverts handled
dispositions to default in place.

Exec
----

An exec *path* resolves in two tiers. First, prelinked images: a lookup
through ``posix_spawn_image_lookup()`` (a weak function applications and test
suites override). Second, with :kconfig:option:`CONFIG_POSIX_EXEC_LLEXT`,
real executable loading: the path is loaded from the filesystem as a linkable
loadable extension (llext), and its exported ``main()`` runs as the new
process image - the image ABI is ``int main(int argc, char **argv, char
**envp)``, and the return value becomes the process's exit status.

Either way :c:func:`execve` replaces the image in place: it aborts every
other member thread with ``k_process_prune()`` and continues on the calling
thread, preserving the process's identity, parent, and group membership;
handled signal dispositions revert to default and ``FD_CLOEXEC`` descriptors
are closed. An extension image cannot unload itself - its caller executes
from the extension - so a table sized by
:kconfig:option:`CONFIG_POSIX_EXEC_LLEXT_MAX` records which extension backs
which exec'd process: a ``main()`` that returns unloads in place, an image
terminated by ``_exit()`` or a signal is unloaded by the ``wait()`` family
when the process is reaped, and a chain-exec'ing image is unloaded by its
successor past exec's point of no return. The new image then runs on the
calling thread, restarted in place. The argument
and environment vectors are staged at the top of the thread's own stack
(``sys_thread_stack_stage()``, bounded by
:kconfig:option:`CONFIG_POSIX_EXEC_ARG_BYTES` else ``E2BIG`` - the
``setup_arg_pages()`` analog), and ``sys_thread_restart()`` resets the
stack pointer just below them and enters the image
(``arch_stack_jump()``, the ``start_thread()`` analog) - same thread,
same stack, pid, signal mask, and priority all trivially preserved.
Memory protection composes per class: MMU platforms map kernel RAM
executable and PMP does not constrain machine-mode execution, so both
run images as-is; XIP MPU platforms map RAM execute-never, so under
``CONFIG_USERSPACE`` the image's llext partitions enter a per-image
memory domain that the exec'ing thread joins before the jump.
Remaining deviations: architectures without stack-jump support
(native_sim, whose threads run on host stacks) run the image on the
calling thread's live frames instead, and a child that is never
collected by ``wait()`` holds its extension-table slot until the slot
is swept on a later exec.

Known deviations
----------------

The following deviations from strict POSIX are documented for
``POSIX_MULTI_PROCESS`` and ``POSIX_SPAWN`` in the current implementation.
Each is expected to be retired as the corresponding substrate lands.

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Interface
     - Deviation
   * - :c:func:`fork`
     - Requires :kconfig:option:`CONFIG_PROCESS_VM` and a kernel-assisted
       resume architecture (x86-64, x86, aarch64); returns ``ENOTSUP``
       otherwise. ``CONFIG_PROCESS_VM`` excludes
       :kconfig:option:`CONFIG_CURRENT_THREAD_USE_TLS` (the current thread
       resolves through the per-CPU arch path instead), since kernel-mode
       reads of user TLS would alias the parent's physical memory through
       the shared kernel page tables.
   * - :c:func:`execve` family
     - A path names a prelinked image or, with
       :kconfig:option:`CONFIG_POSIX_EXEC_LLEXT`, a filesystem ELF
       extension; the new image runs on the calling thread restarted at
       the base of its own stack (on its live frames only where the
       architecture lacks stack-jump support, e.g. native_sim).
       ``execvp``/``execlp`` resolve bare names against
       :kconfig:option:`CONFIG_POSIX_EXEC_PATH_PREFIX` rather than a
       ``PATH`` environment variable.
   * - Kernel-staged user memory in a fork child
     - The kernel stages user memory through the shared kernel page tables,
       which map the child's user addresses to the parent's frames. A fork
       child must therefore use scalar-only system calls (no pointer
       copy-outs) and cannot take user-mode signal-handler delivery (the
       trampoline frame is kernel-staged); an inherited ignored disposition
       is honored, since discard happens kernel-side. Lifted when copy-outs
       learn to resolve through the child's mappings.

.. _posix_implementation_signals:

Signal Implementation Details
=============================

The :ref:`POSIX_SIGNALS <posix_option_group_signals>` option group is implemented on top of the
kernel signal API (``k_sig_*``). Zephyr does not yet conventionally support interrupted or
restarted system calls. As a result, signals are delivered to a thread on its return from a
system call, so a thread that never enters the kernel does not run a signal handler
:ref:`†<posix_undefined_behaviour>`.

.. note::
    Efforts to support the standard behaviour (interrupting blocking system calls, with
    ``EINTR`` and optional restart semantics) are underway.

``abort()``, ``raise()``, and ``signal()`` are the three functions of the option group that ISO C
also requires, so they must work in a freestanding-plus-ISO-C environment where
:kconfig:option:`CONFIG_POSIX_SIGNALS` is not selected. They are implemented once, in the common
C library behind :kconfig:option:`CONFIG_COMMON_LIBC_SIGNAL`, and behave identically whichever
standard they are reached through: the calling thread is the same thread in ISO C, POSIX, and the
kernel. When the option group is linked, weak references pick up its signal number table and its
delivery shim, so ``signal()`` makes the same kernel registration :c:func:`sigaction` makes and a
disposition installed through either is visible to the other. Without it, only the six signal
numbers ISO C defines (``SIGABRT``, ``SIGFPE``, ``SIGILL``, ``SIGINT``, ``SIGSEGV``, and
``SIGTERM``) are reachable, and those coincide with the kernel's numbering, so handlers take
delivery directly.

Note that only the six ISO C signal numbers can be assumed: a C library is free to number every
other signal however it likes, and the numbering shipped with a given toolchain need not match
the Linux-aligned numbering used by the kernel and the POSIX option group.

Process support (:ref:`POSIX_MULTI_PROCESS <posix_option_group_multi_process>`, over the
kernel's :kconfig:option:`CONFIG_PROCESS` thread-group substrate - see
:ref:`posix_multi_process_design`) is optional, which shapes the implementation in several ways:

Signals are process-directed or thread-directed
   :c:func:`kill` and :c:func:`sigqueue` are process-directed and ``pthread_kill()`` is
   thread-directed. With process support, a positive ``pid`` names a process, ``0`` the caller's
   process group, a negative ``pid`` another process group, and ``-1`` every other process
   (excluding the reserved system processes and the caller's own); a process-directed signal is
   delivered to one member thread that has it unblocked, or pends at process scope until a member
   unblocks or waits for it. In a single-process configuration the process's own pid, ``0``, and
   ``-1`` all name the single (figurative) process, with the same process-directed delivery: the
   caller takes the signal when it has it unblocked, and it otherwise pends at system scope for
   the first thread that unblocks or waits for it.

Dispositions are per-process with process support
   With :kconfig:option:`CONFIG_PROCESS`, a signal action is a property of the process, as POSIX
   specifies: installed by any member thread, in force for every member, and outliving its
   installer. A forked child inherits every disposition, a freshly created process image inherits
   only ignored dispositions (fork+exec semantics), and ``exec`` reverts handled dispositions to
   default. Without process support the action database is keyed by (signal, thread), so an
   action installed by one thread is not in force for another and dies with its installer.

Kernel threads block all signals by default
   A kernel thread must opt in to signal delivery, with :c:func:`sigprocmask` or
   ``pthread_sigmask()``, before any signal can be delivered to it. User-mode threads start with
   no signals blocked, in line with the POSIX specification.

Faults are not signals
   A CPU exception or kernel error is reported through the fatal error path rather than by
   generating ``SIGILL``, ``SIGFPE``, ``SIGSEGV``, or ``SIGBUS`` for the offending thread.

Elastipool: Elastic Object Pools
=================================

Every 1:1 mapping requires a pool of kernel objects from which to allocate. Zephyr uses
**elastipool** (``<zephyr/sys/elastipool.h>``) — an elastic object pool that bridges the gap
between guaranteed static allocation and on-demand dynamic growth.

An elastipool instance is parameterized by two values:

- **min** — the number of objects pre-allocated in a static array at compile time (guaranteed to
  be available, zero-latency allocation via bitmap).
- **max** — the upper bound on total objects. When ``max > min``, up to ``max − min`` additional
  objects may be allocated from the heap (or other desired memory pool) at runtime.

It is useful on larger systems (e.g., those with an MMU) where the exact number of required
objects is not known at compile time.

The relationship between ``min`` and ``max`` selects one of three operational modes:

Static-only pools (``min == max``)
----------------------------------

When ``min`` equals ``max``, the pool uses only statically allocated objects. No heap is required.
Allocation and deallocation are O(1) bitmap operations.

.. graphviz::
   :caption: Static-only pool (min == max)

   digraph {
       rankdir=LR;
       node [shape=record, style=filled];

       subgraph cluster_pool {
           label="Static Pool (min == max)";
           style=filled;
           color="#fff3e0";
           fillcolor="#fff3e0";
           bmp [label="Bitmap" fillcolor="#ffe0b2"];
           obj [label="{obj[0]|obj[1]|...|obj[min−1]}" fillcolor="#ffcc80"];
           bmp -> obj [label="index"];
       }
   }

This is the most deterministic mode. It is appropriate for safety-critical or memory-constrained
systems where heap allocation is undesirable or unavailable.

Dynamic-only pools (``min == 0``)
---------------------------------

When ``min`` is zero, all objects are allocated from the heap. A hash map tracks outstanding
allocations so that ``free`` and ``check`` operations can validate pointers.

.. graphviz::
   :caption: Dynamic-only pool (min == 0)

   digraph {
       rankdir=LR;
       node [shape=record, style=filled];

       subgraph cluster_pool {
           label="Dynamic Pool (min == 0)";
           style=filled;
           color="#e8eaf6";
           fillcolor="#e8eaf6";
           heap [label="Heap\n(aligned_alloc)" fillcolor="#c5cae9"];
           map  [label="Hash Map\n(pointer tracking)" fillcolor="#c5cae9"];
           heap -> map [label="register"];
       }
   }

This mode requires ``CONFIG_SYS_HASH_MAP`` and ``CONFIG_SYS_HASH_FUNC32``.

Hybrid pools (``0 < min < max``)
--------------------------------

When both ``min`` and ``max`` are non-zero and ``max > min``, the pool operates in hybrid
("elastic") mode. Allocation first attempts the static bitmap; only when the static slab is
exhausted does it fall through to the heap.

.. graphviz::
   :caption: Hybrid (elastic) pool (0 < min < max)

   digraph {
       rankdir=TB;
       node [shape=record, style=filled];

       alloc [label="sys_elastipool_alloc()" shape=ellipse fillcolor="#e0f7fa"];

       subgraph cluster_static {
           label="Static Slab";
           style=filled;
           color="#e8f5e9";
           fillcolor="#e8f5e9";
           bmp [label="Bitmap" fillcolor="#c8e6c9"];
           slab [label="{obj[0]|...|obj[min−1]}" fillcolor="#a5d6a7"];
       }

       subgraph cluster_dynamic {
           label="Dynamic Allocation";
           style=filled;
           color="#fce4ec";
           fillcolor="#fce4ec";
           heap [label="Heap" fillcolor="#f8bbd0"];
           map  [label="Hash Map" fillcolor="#f8bbd0"];
       }

       alloc -> bmp;
       bmp -> slab [label="index"];
       alloc -> heap [style=dashed];
       heap -> map [label="register" style=dashed];
   }

This mode gives the best of both worlds: guaranteed availability of the first ``min`` objects with
the ability to allocate up to ``max``.

In the threading subsystem, the pools are instantiated in ``zephyr/lib/os/thread.c``:

.. code-block:: c

   K_MUTEX_ARRAY_DEFINE(sys_mutex_pool, SYS_THREAD_MUTEX_MIN);
   SYS_ELASTIPOOL_DEFINE_ADVANCED(mutex_pool,
       sizeof(struct k_mutex), __alignof(struct k_mutex),
       SYS_THREAD_MUTEX_MIN, CONFIG_SYS_THREAD_MUTEX_MAX,
       mutex_pool_heap_alloc, sys_mutex_pool, static);

Distributed Kconfig
===================

A recurring problem in embedded systems is knowing *at compile time* how many of a given resource
the final application will need. Different subsystems, libraries, and tests each require some
number of mutexes, threads, stacks, etc.

Zephyr solves this with **distributed Kconfig variables**: each subsystem declares its own
``CONFIG_SYS_THREAD_<POOL>_MIN_ADD_<SUBSYSTEM>`` symbol that contributes to the total minimum
pool size. At build time, CMake sums all ``_MIN_ADD_*`` contributions together with the base
``CONFIG_SYS_THREAD_<POOL>_MIN`` value and emits a single ``SYS_THREAD_<POOL>_MIN`` compile
definition.

.. graphviz::
   :caption: Distributed Kconfig aggregation for SYS_THREAD_MUTEX_MIN

   digraph {
       rankdir=LR;
       node [shape=rect, style="filled,rounded"];

       app   [label="Application\nCONFIG_SYS_THREAD_MUTEX_MIN=2" fillcolor="#c8e6c9"];
       test  [label="Test suite\n_MIN_ADD_TEST=4"                 fillcolor="#bbdefb"];
       lib   [label="Library\n_MIN_ADD_MYLIB=1"                   fillcolor="#ffe0b2"];

       sum   [label="CMake\nΣ = 2 + 4 + 1 = 7" shape=ellipse     fillcolor="#f3e5f5"];
       def   [label="SYS_THREAD_MUTEX_MIN=7"                      fillcolor="#d1c4e9"];

       app  -> sum;
       test -> sum;
       lib  -> sum;
       sum  -> def;
   }

The aggregation is performed by this CMake loop in ``zephyr/lib/os/CMakeLists.txt``:

.. code-block:: cmake

   foreach(_pool CONDVAR MUTEX STACK THREAD)
     import_kconfig(
       CONFIG_SYS_THREAD_${_pool}_MIN_ADD_
       ${DOTCONFIG}
       _sys_thread_${_pool}_min_add_keys
     )
     set(_min ${CONFIG_SYS_THREAD_${_pool}_MIN})
     foreach(_add ${_sys_thread_${_pool}_min_add_keys})
       math(EXPR _min "${_min} + ${${_add}}")
     endforeach()
     zephyr_compile_definitions(
       SYS_THREAD_${_pool}_MIN=${_min}
     )
   endforeach()

The result is a non-``CONFIG_`` prefixed compile definition (e.g., ``SYS_THREAD_MUTEX_MIN=7``)
that is used to size the static portion of the corresponding elastipool. The ``CONFIG_``-prefixed
``_MAX`` value (e.g., ``CONFIG_SYS_THREAD_MUTEX_MAX``) sets the upper bound.

This pattern has several advantages:

- **Decentralized** — each library or test declares exactly what it needs; no central manifest to
  maintain.
- **Additive** — contributions are summed, so adding a new subsystem cannot reduce the pool below
  what existing consumers require.
- **Extensible** — the same ``import_kconfig`` / sum / ``zephyr_compile_definitions`` pattern is
  already used for file descriptors (``ZVFS_OPEN_ADD_SIZE_*``) and is expected to expand to
  other bounded resources in the future.

To add a new contributor, create a Kconfig symbol in your subsystem:

.. code-block:: kconfig

   config SYS_THREAD_MUTEX_MIN_ADD_MYLIB
       int "Mutexes required by mylib"
       default 3

Then set it in your ``prj.conf`` or ``testcase.yaml``:

.. code-block:: cfg

   CONFIG_SYS_THREAD_MUTEX_MIN_ADD_MYLIB=3

The build system automatically discovers all ``_MIN_ADD_*`` symbols and includes them in the sum.

POSIX Timers
============

POSIX timers map ~1:1 onto kernel timers: ``timer_t`` handles are :c:struct:`k_timer` objects
allocated from the OS-managed system timer pool (:kconfig:option:`CONFIG_SYS_TIMER`), armed at
full tick resolution with the clock-based kernel timer APIs
(:kconfig:option:`CONFIG_TIMER_CLOCK`), and validated as kernel objects in user mode - a stale
or foreign ``timer_t`` faults instead of corrupting memory.

**Allocation.** Applications and libraries reserve guaranteed, statically-allocated timers by
defining int Kconfig symbols named ``CONFIG_SYS_TIMER_MIN_ADD_<NAME>``; the build system sums
them with :kconfig:option:`CONFIG_SYS_TIMER_MIN`. Up to
:kconfig:option:`CONFIG_SYS_TIMER_MAX` (default ``INT_MAX``) timers in total may be created,
the excess allocated dynamically without guarantees; setting the maximum equal to the
accumulated minimum prohibits dynamic allocation entirely. :c:func:`timer_create` reports pool
exhaustion as ``EAGAIN``.

**Expiry notification** is signal-based (never a callback in interrupt context, so it is
robust for user-mode callers) with POSIX one-pending semantics: at most one expiry signal per
timer is queued at a time, further expiries are accounted as overruns, and
:c:func:`timer_getoverrun` reads the count latched at the most recent delivery,
non-destructively, computed from the exact scheduled-expiry grid (expiries coalesced by
tickless wakeups are counted correctly). ``SIGEV_THREAD`` maps onto the kernel's
function-notification dispatch (see :c:member:`k_timer_notify.fn`): each expiry runs the
notification function in a fresh detached thread, as POSIX specifies, spawned by a single
kernel dispatcher woken through a reserved signal number just past ``SIGRTMAX`` that
applications can neither send, mask, nor wait on. ``sigev_notify_attributes`` are translated
at :c:func:`timer_create` time - stack size and priority are honored, the detach state is
always detached - and the caller may destroy the attribute object afterwards; no
per-timer allocation is made by the POSIX layer. :c:func:`timer_getoverrun` is valid
inside the notification function. The Linux-compatible ``SIGEV_THREAD_ID`` extension is
available under ``_GNU_SOURCE`` (the ``sigev_notify_thread_id`` member carries a
``pthread_t`` value). A ``NULL`` ``evp`` behaves as POSIX specifies: ``SIGEV_SIGNAL`` with
``SIGALRM`` and the timer ID as the value.

**Clocks.** ``TIMER_ABSTIME`` deadlines are honored on both ``CLOCK_MONOTONIC`` and
``CLOCK_REALTIME``; with :kconfig:option:`CONFIG_TIMER_REALTIME` (selected by
:kconfig:option:`CONFIG_POSIX_TIMERS`), armed absolute ``CLOCK_REALTIME`` timers are
re-targeted when :c:func:`clock_settime` moves the wall clock - a forward jump past the
deadline fires the timer immediately, a backward jump defers it. Re-targeting applies to the
initial expiry only; afterwards a periodic timer's interval continues on the monotonic grid,
matching Linux.

**Reduced mode.** When signal-based expiry notification
(:kconfig:option:`CONFIG_TIMER_SIGNAL`) is unavailable, ``SIGEV_NONE`` timers remain fully
functional as time sources, while ``SIGEV_SIGNAL``, ``SIGEV_THREAD``, and
``SIGEV_THREAD_ID`` report ``ENOTSUP`` from :c:func:`timer_create` (``SIGEV_THREAD``
additionally requires the system thread pool, :kconfig:option:`CONFIG_SYS_THREAD` with
:kconfig:option:`CONFIG_THREAD_DETACH`). All AEP profiles select the signal subsystem.

Migration from earlier releases: :kconfig:option:`CONFIG_POSIX_TIMER_MAX` (still reported by
``TIMER_MAX`` and ``sysconf(_SC_TIMER_MAX)``) is no longer user configurable - it is derived
from :kconfig:option:`CONFIG_SYS_TIMER_MAX`, which bounds the pool; guaranteed capacity moved
to the distributed ``CONFIG_SYS_TIMER_MIN_ADD_<NAME>`` minimum, and
``CONFIG_TIMER_CREATE_WAIT`` was removed because :c:func:`timer_create` no longer blocks.


POSIX Message Queues
====================

POSIX message queues map 1:1 onto kernel message queues: a queue is a :c:struct:`k_msgq`
object allocated from the OS-managed system message queue pool
(:kconfig:option:`CONFIG_SYS_MSGQ`), and ``mqd_t`` is a file descriptor naming it. Every
``mq_*()`` call is one system call plus ``errno`` translation, so the whole option group is
usable from user mode: no kernel object pointer is ever exposed, and a stale or foreign
descriptor reports ``EBADF`` instead of corrupting memory.

**Allocation.** Applications and libraries reserve guaranteed, statically-allocated queues by
defining int Kconfig symbols named ``CONFIG_SYS_MSGQ_MIN_ADD_<NAME>``; the build system sums
them with :kconfig:option:`CONFIG_SYS_MSGQ_MIN`. Up to :kconfig:option:`CONFIG_SYS_MSGQ_MAX`
(default ``INT_MAX``) queues may exist, the excess allocated dynamically without guarantees;
setting the maximum equal to the accumulated minimum prohibits dynamic allocation entirely.
Statically allocated queues draw message storage from a fixed per-queue budget
(:kconfig:option:`CONFIG_SYS_MSGQ_BUF_SIZE`) and :c:func:`mq_open` reports a geometry that
does not fit as ``ENOSPC``; dynamically allocated queues size their storage to the request and
are not bound by that budget. Queue names are at most
:kconfig:option:`CONFIG_SYS_MSGQ_NAMELEN_MAX` characters.

**Messages** carry a priority and a length. :c:func:`mq_send` orders messages by descending
priority, FIFO within a priority, and :c:func:`mq_receive` reports the priority of the message
it returns along with its actual length - messages shorter than ``mq_msgsize`` are delivered
as sent, not padded. Priorities range from ``0`` to
:kconfig:option:`CONFIG_POSIX_MQ_PRIO_MAX` - 1 (reported by ``MQ_PRIO_MAX``).

**Descriptors.** The access mode and ``O_NONBLOCK`` are per open file description, as POSIX
specifies: two descriptors for one queue may differ in both, and :c:func:`mq_setattr` changes
``O_NONBLOCK`` for the calling descriptor alone. A queue persists after its last descriptor is
closed and is destroyed only once it has been unlinked and no descriptor remains; the name is
released immediately by :c:func:`mq_unlink`, so it may be reused for a new queue while the old
one is still being drained. Both timed calls take absolute ``CLOCK_REALTIME`` deadlines,
report expiry as ``ETIMEDOUT``, and are distinguished from a non-blocking descriptor's
``EAGAIN``.

**Notification.** :c:func:`mq_notify` supports ``SIGEV_NONE``, ``SIGEV_SIGNAL``,
``SIGEV_THREAD``, and (under ``_GNU_SOURCE``) the Linux-compatible ``SIGEV_THREAD_ID``. The
registration is armed in the kernel, so the empty-to-non-empty transition is detected
atomically with the send rather than by sampling the queue depth around it; it fires exactly
once and is consumed as it fires, after which a new registration may be armed. Arming while
one is already armed reports ``EBUSY``; removing one that was never armed succeeds, matching
Linux. ``SIGEV_SIGNAL`` targets the registering thread rather than the process
:ref:`†<posix_undefined_behaviour>` and delivers ``si_code`` ``SI_MESGQ``. ``SIGEV_THREAD`` maps onto the kernel's function
notification dispatch (see :c:member:`sys_msgq_notify.fn`): each arrival runs the
notification function in a fresh detached system-pool thread, spawned by a kernel dispatcher
woken through a reserved signal number past ``SIGRTMAX`` that applications can neither send,
mask, nor wait on - notification threads for registrations made by user threads run in user
mode, in the registrant's memory domain, with the registrant's object permissions.
``sigev_notify_attributes`` are translated at registration time - stack size and priority are
honored, the detach state is always detached - and the caller may destroy the attribute
object afterwards; no POSIX-side state or service thread is involved, so every notification
form works from user mode.

Migration from earlier releases: ``mqd_t`` is now an ``int`` file descriptor rather than an
opaque pointer, so a failed :c:func:`mq_open` compares against ``(mqd_t)-1`` and descriptors
count against the ZVFS descriptor table
(:kconfig:option:`CONFIG_ZVFS_OPEN_ADD_SIZE_SYS_MSGQ`). :c:func:`mq_receive` and
:c:func:`mq_timedreceive` return ``ssize_t``, as POSIX specifies.
:kconfig:option:`CONFIG_POSIX_MQ_OPEN_MAX`, ``CONFIG_MSG_SIZE_MAX``, and
``CONFIG_MQUEUE_NAMELEN_MAX`` are no longer user configurable - they derive from the
corresponding ``SYS_MSGQ`` bounds.

POSIX Asynchronous I/O
======================

Asynchronous I/O is a thin veneer over ``sys_aio``, an OS-managed pool of asynchronous I/O
requests (:kconfig:option:`CONFIG_SYS_AIO`) performed by a dedicated kernel service queue.
:c:struct:`aiocb` carries the request handle, and every ``aio_*()`` call is one system call
plus ``errno`` translation - no POSIX-side request state or service thread exists - so the
whole option group works from user mode. Handles are validated by pool membership on every
call: a stale or foreign handle reports ``EINVAL`` instead of corrupting memory.

**Dispatch.** Submission classifies the descriptor once: descriptors with a waitable
readiness condition (sockets, eventfds) are armed on ``ZVFS_POLLIN``/``ZVFS_POLLOUT``
readiness with a triggered work item over the events their backend registers for
:c:func:`poll`, so no thread parks while a request waits; always-ready descriptors (regular
files - see below), already-ready ones, and offloaded sockets execute directly on the
service queue thread. Operations execute one at a time in completion order
(:kconfig:option:`CONFIG_SYS_AIO_WORKQ_PRIO`,
:kconfig:option:`CONFIG_SYS_AIO_WORKQ_STACK_SIZE`): a slow operation delays those queued
behind it, the file-system driver runs on the service queue's stack, and a request whose
readiness regresses between the wakeup and the transfer may briefly block the queue. This
dispatcher is deliberately a small internal seam: a future backend with uniform,
driver-level asynchronous submission can replace it without changing the system call
contract or the POSIX layer above it. Connecting POSIX asynchronous I/O to :ref:`RTIO
<rtio>` is in the early planning stages - RTIO would first need to express operations on
integer file descriptors and file offsets (today its submissions address ``iodev`` objects
with no offset field), possibly by way of a ``posix_devctl()``-style uniform device
interface.

As a prerequisite, :c:func:`poll` was taught that descriptors whose backends have no poll
support - regular files, shared memory, message queues, and the standard streams - are
always ready for input and output, as POSIX requires of regular files, rather than failing
with an unspecified error.

**Completion** is recorded in the request and is sticky: :c:func:`aio_error` reports
``EINPROGRESS`` until the operation finishes, :c:func:`aio_suspend` waits any-of on
per-request completion signals (at most :kconfig:option:`CONFIG_SYS_AIO_WAIT_MAX` entries),
and :c:func:`aio_return` reaps the request, returning its slot to the pool. Closing a
descriptor with requests in flight completes them with ``EBADF``: the service queue
re-validates the descriptor's backing object before every transfer. :c:func:`aio_cancel`
removes armed and queued requests race-free - a claimed request completes with
``ECANCELED``, wakes waiters, and still fires its notification - and reports
``AIO_NOTCANCELED`` for one already executing.

**Notification** mirrors the message queue model: ``SIGEV_SIGNAL`` generates the signal
kernel-side with an ``SI_ASYNCIO`` code (the target is validated at submission, so no
sender permissions apply at completion time), and ``SIGEV_THREAD`` runs the notification
function in a fresh detached system-pool thread - in user mode, in the submitter's memory
domain, when the request was submitted from a user thread. Completion groups back
:c:func:`lio_listio`'s ``LIO_NOWAIT`` list notification: each submission joins the group,
and the group's single notification fires the moment its last member completes, after which
the group destroys itself.

**Allocation** follows the distributed-minimum idiom shared with ``sys_thread``,
``sys_timer``, and ``sys_msgq``: ``CONFIG_SYS_AIO_MIN_ADD_<NAME>`` contributions are summed
with :kconfig:option:`CONFIG_SYS_AIO_MIN` into a statically allocated, guaranteed pool
minimum, with :kconfig:option:`CONFIG_SYS_AIO_MAX` bounding the heap-allocated remainder.
``AIO_MAX`` and ``AIO_LISTIO_MAX`` derive from those bounds
(:kconfig:option:`CONFIG_POSIX_AIO_MAX`, :kconfig:option:`CONFIG_POSIX_AIO_LISTIO_MAX`).
:c:func:`aio_fsync` synchronizes data and metadata together: ``O_DSYNC`` and ``O_SYNC`` are
equivalent.

**Prioritization**: with :kconfig:option:`CONFIG_POSIX_PRIORITIZED_IO` (enabled by default;
selects :kconfig:option:`CONFIG_SYS_AIO_PRIORITY`), ``_POSIX_PRIORITIZED_IO`` is defined and
the service queue performs ready requests in priority order from a min-heap rather than in
submission order: each request is ordered by the calling thread's scheduling priority lowered
by ``aio_reqprio`` (0 through ``AIO_PRIO_DELTA_MAX``, one Zephyr priority level per unit),
first-in-first-out among equal priorities, for every descriptor served by the request pool.
:ref:`posix_scheduling_priorities` explains why that ordering is a min-heap in Zephyr's
native priority space.
When disabled, ``AIO_PRIO_DELTA_MAX`` is 0, ``aio_reqprio`` must be 0, and ready requests
are performed in submission order.

File System Implementation Details
==================================

The :ref:`POSIX_FILE_SYSTEM <posix_option_group_file_system>` option group is a thin layer over
the Zephyr Virtual File System (ZVFS) and, through it, the :ref:`file system API <file_system_api>`.
Path, directory-stream, and working-directory operations are ZVFS system calls, so the whole
option group works from user mode when :kconfig:option:`CONFIG_USERSPACE` is enabled.

ZVFS is the common waypoint for every file-descriptor consumer. The POSIX file API and the C
library's ``<stdio.h>`` both bottom out in the same ZVFS entry points - ``fopen()`` and
``open()`` are two spellings of ``zvfs_open()`` - so an application reaches one file system
implementation whichever interface it uses.

.. graphviz::
   :caption: libc and the POSIX API both reach the file system through ZVFS

   digraph {
       node [shape=rect, style="rounded,filled"];
       rankdir=TB;

       app        [label="application", fillcolor="#ffffff"];
       libc       [label="C library\n(fopen, fread, ...)", fillcolor="#dae8fc"];
       posix      [label="POSIX file API\n(open, stat, opendir, ...)", fillcolor="#d5e8d4"];
       zvfs       [label="ZVFS\n(fd table + per-fd vtable)", fillcolor="#ffe6cc"];
       fs         [label="file system subsystem\n(fs_open, fs_readdir, ...)", fillcolor="#f8cecc"];
       backend    [label="FatFs / LittleFS / ext2 / ...", fillcolor="#f8cecc"];

       app -> libc;
       app -> posix;
       libc -> zvfs;
       posix -> zvfs;
       zvfs -> fs;
       fs -> backend;
   }

The file system is only one of several descriptor types ZVFS multiplexes. Every open descriptor
is a slot in the ZVFS fd table carrying a vtable; a read, write, or ``ioctl`` on the descriptor
dispatches through that vtable to the owning backend, so the same ``read()``/``write()``/
``close()``/``poll()`` reach a regular file, a socket, an eventfd, or a message queue without the
caller knowing which.

.. graphviz::
   :caption: A ZVFS descriptor dispatches through its vtable to one of many backends

   digraph {
       node [shape=rect, style="rounded,filled"];
       rankdir=TB;

       fd    [label="ZVFS fd (K_OBJ_FILE slot + fd_op_vtable)", fillcolor="#ffe6cc"];
       file  [label="regular file\n(file system subsystem)", fillcolor="#f8cecc"];
       sock  [label="socket\n(network stack)", fillcolor="#dae8fc"];
       efd   [label="eventfd", fillcolor="#d5e8d4"];
       mq    [label="message queue", fillcolor="#e1d5e7"];

       fd -> file;
       fd -> sock;
       fd -> efd;
       fd -> mq;
   }

Zephyr kernel caveats
---------------------

These reflect the Zephyr kernel model rather than the file system.

Working directory
   The current working directory is a single, system-wide string maintained by ZVFS. In a
   single-process configuration that matches the POSIX per-process model exactly; with
   :ref:`POSIX_MULTI_PROCESS <posix_option_group_multi_process>` it is shared by all processes
   rather than copied into a child at :c:func:`fork` :ref:`†<posix_undefined_behaviour>`.
   Every path operation resolves its
   argument against it, so relative paths work throughout - including through ISO C
   :c:func:`fopen` and POSIX :c:func:`open`, which share the same ZVFS entry point.
   :c:func:`chdir`, :c:func:`fchdir`, and :c:func:`getcwd` read and update it. Resolution is
   purely lexical: ``.``, ``..``, and repeated separators are collapsed without consulting the
   file system. On a backend that supports symbolic links (such as ext2) this is a simplification
   - full POSIX pathname resolution would resolve a ``..`` component relative to the target of a
   preceding symbolic link, whereas lexical resolution collapses it textually.

Temporary files
   :c:func:`tmpfile` and :c:func:`tmpnam` are provided by the common C library
   (:kconfig:option:`CONFIG_COMMON_LIBC_TMPFILE`) for every libc, and require ``/tmp`` to exist
   on a mounted file system. ISO C removes the :c:func:`tmpfile` stream when it is closed or at
   normal program termination; the file is removed on the final close of its descriptor -
   :c:func:`fclose`, or the descriptor sweep at process exit when process support
   (:kconfig:option:`CONFIG_PROCESS`) is enabled. Without process support a stream that is never
   closed leaves its file in place, as there is no process termination at which to remove it.

Zephyr FS subsystem caveats
---------------------------

Because the POSIX layer is thin, its capabilities track the Zephyr FS subsystem - the
``fs_*`` API and its ``fs_file_system_t`` back-end interface - rather than the on-disk features
of any particular file system. The deviations below are gaps in what the subsystem exposes, not in
the POSIX code or the backing file system; when the subsystem grows an operation, the POSIX layer
surfaces it with little or no change.

No hard links
   The Zephyr FS subsystem exposes no hard-link operation, so :c:func:`link` always fails with
   ``EPERM`` - which POSIX permits for an implementation that prohibits the operation - and
   ``st_nlink`` is always 1. This is a subsystem limitation, not a file system one: ext2, for
   example, stores hard links on disk, but the subsystem offers no way to create or count them.

No symbolic links
   The subsystem exposes no symbolic-link operations, so path resolution never follows a symlink
   and stays purely lexical (see *Working directory* above). As with hard links this is a subsystem
   limitation, not a file system one - ext2 stores symbolic links on disk, but the subsystem offers
   no way to create or read them.

Permission bits are not exposed
   The subsystem's ``struct fs_dirent`` carries no mode bits, so :c:func:`stat` cannot report a
   file system's stored permissions even when it has them - ext2 again being the example. It
   reports a synthesised mode instead: read and search for everyone, plus write unless the mount
   is read-only. :c:func:`access` and :c:func:`mkdir` follow suit - ``mkdir()`` accepts its
   ``mode`` argument and ignores it, and :c:func:`access` grants ``X_OK`` only for directories.

Timestamps
   File timestamps are surfaced only when the backend records them, gated by
   :kconfig:option:`CONFIG_FS_DIRENT_EXT`. Where a backend stores no timestamps, ``fs_utime()``
   returns ``-ENOTSUP`` - which :c:func:`utime` surfaces as ``errno`` ``ENOTSUP`` - and
   :c:func:`stat` reports zero for ``st_atim``/``st_mtim``/``st_ctim``. Per-backend behaviour is
   noted under the file-system-specific caveats below.

FAT-specific caveats
--------------------

FatFs (the ELM ChaN library behind :kconfig:option:`CONFIG_FAT_FILESYSTEM_ELM`) predates and
ignores POSIX naming conventions, which leaks through the abstraction in a few ways worth knowing
when FatFs is the backend.

Type-name collision with ``<dirent.h>``
   FatFs declares its public types with unprefixed names - ``DIR``, ``FIL``, ``FATFS``,
   ``FRESULT`` - in ``ff.h``. ``DIR`` collides with the POSIX ``DIR`` from ``<dirent.h>``, so a
   single translation unit cannot include both ``ff.h`` (needed to declare a ``FATFS`` mount
   object, for example) and ``<dirent.h>``. Code that mounts a FAT volume *and* walks directories
   with POSIX :c:func:`opendir`/:c:func:`readdir` must split those uses across separate
   translation units, or use the ``fs_*`` directory API on the FatFs side. This is a namespacing
   defect in the FatFs module, not in the POSIX layer, but the POSIX layer inherits the
   constraint.

Short file names
   Without :kconfig:option:`CONFIG_FS_FATFS_LFN`, FatFs is limited to 8.3 short names, so
   ``pathconf(_PC_NAME_MAX)`` reports 12 and longer names fail. Short names are stored
   upper-cased, so :c:func:`readdir` returns ``FILE.TXT`` for a file created as ``file.txt``.

Coarse, modification-only timestamps
   FAT records a single modification time with two-second granularity and no access time, so
   ``st_atim`` mirrors ``st_mtim`` and ``pathconf(_PC_TIMESTAMP_RESOLUTION)`` reflects the coarser
   value. Timestamp writes additionally require :kconfig:option:`CONFIG_FS_FATFS_FF_USE_CHMOD`.

Non-canonical ``errno`` values
   FatFs maps several conditions onto ``errno`` values that differ from the POSIX-preferred
   spelling: opening a non-directory as a directory reports ``ENOENT`` rather than ``ENOTDIR``,
   and removing a non-empty directory reports ``EACCES`` rather than ``ENOTEMPTY``. Tests that
   assert these paths accept either spelling.

rename() is not an atomic replace
   FatFs :c:func:`rename` fails with ``EEXIST`` when the target already exists, rather than
   atomically replacing it as POSIX requires.

LittleFS-specific caveats
-------------------------

No timestamps
   LittleFS records no per-file timestamps, so :c:func:`utime` fails with ``ENOTSUP`` and
   :c:func:`stat` reports zero for ``st_atim``/``st_mtim``/``st_ctim``.

ext2-specific caveats
---------------------

Among the in-tree backends, ext2 is the notable case where the on-disk format *does* support the
features the Zephyr FS subsystem withholds. The limitations in *Zephyr FS subsystem caveats* above
therefore still apply to ext2, even though the volume itself could satisfy them:

Hard links are not supported
   :c:func:`link` fails with ``EPERM`` and ``st_nlink`` is always 1, though ext2 stores hard links
   on disk.

Symbolic links are not supported
   Symbolic links are neither created nor followed, though ext2 stores them on disk.

Permissions may not be reported accurately
   :c:func:`stat` reports a synthesised mode rather than the file's stored permission bits.
