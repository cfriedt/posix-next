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

The general rule is that Option Groups will _always_ have an associated Kconfig option and _some_
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
kernel signal API (``k_sig_*``). Signals are delivered to a thread on its return from a system
call, so a thread that never enters the kernel never runs a signal handler.

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
