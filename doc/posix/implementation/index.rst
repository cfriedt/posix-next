.. _posix_details:

Implementation Details
######################

In many ways, Zephyr provides support like any POSIX OS; API bindings are provided in the C
programming language, POSIX headers are available in the standard include path, when configured.

Unlike other multi-purpose POSIX operating systems

- Zephyr is not "a POSIX OS". The Zephyr kernel was not designed around the POSIX standard, and
  POSIX support is an opt-in feature
- Zephyr apps are not linked separately, nor do they execute as subprocesses
- Zephyr, libraries, and application code are compiled and linked together, running similarly to
  a single-process application, in a single (possibly virtual) address space
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
separately. Zephyr usually maps each standard Option Group to a directory under

- ``lib/posix/``
  - for groups such as C extensions that generally do not require OS involvement, and
- ``lib/posix/options``
  - for features that generally do require OS involvement.

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
``lib/posix/options/``.

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

Zephyr does not support processes, which shapes the implementation in several ways:

No processes
   Zephyr has a single process, so a ``pid_t`` is either the value returned by :c:func:`getpid`,
   which names the calling thread, or the ``pthread_t`` of a specific thread. Sending a signal to
   a process group is not supported and fails with ``ESRCH``.

Dispositions are per-thread
   POSIX associates a signal action with the process, but the kernel action database is keyed by
   (signal, thread), so an action installed by one thread is not in force for another. A thread
   that needs to catch a signal must install the action itself.

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
Linux. ``SIGEV_SIGNAL`` targets the registering thread until Zephyr gains process support and
delivers ``si_code`` ``SI_MESGQ``. ``SIGEV_THREAD`` maps onto the kernel's function
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
