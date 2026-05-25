.. _posix_getting_started:

Getting Started
###############

This guide walks through setting up a workspace, building your first POSIX
application on Zephyr, and running it on a simulator or real hardware.

Prerequisites
*************

- `Zephyr dependencies
  <https://docs.zephyrproject.org/latest/develop/getting_started/index.html#install-dependencies>`_
  installed (CMake, Python 3, device-tree compiler, etc.)

Set up the workspace
********************

.. code-block:: bash

   # Create and activate a Python virtual environment
   python3 -m venv ~/posix-next/.venv
   source ~/posix-next/.venv/bin/activate
   pip install west

   # Create a new west workspace with posix-next as the manifest repo
   west init -m https://github.com/cfriedt/posix-next --mr main ~/posix-next
   cd ~/posix-next
   west update

   # Re-home the manifest repo to modules/lib/posix (to workaround a bug in west)
   mv posix-next modules/lib/posix
   sed -i 's|manifest/path:.*|manifest/path = modules/lib/posix|' .west/config

Install the Zephyr SDK
**********************

If you already have a Zephyr SDK installed, point the appropriate environment
variables to it:

.. code-block:: bash

   export ZEPHYR_BASE=$HOME/posix-next/zephyr
   export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
   export ZEPHYR_SDK_INSTALL_DIR=/path/to/zephyr-sdk

Otherwise, install the SDK using ``west``:

.. code-block:: bash

   west sdk install

Hello, POSIX world!
*******************

A primary goal for Zephyr's POSIX implementation is to ensure that POSIX APIs
can be used portably, with as few source modifications as possible.  This sample
uses automatic thread allocation as well as automatic thread stack allocation to
conform to standard POSIX API usage.

.. literalinclude:: ../../samples/posix/hello_world/src/main.c
   :language: c

Build and run
=============

To build and run on a simulator such as ``native_sim`` or ``qemu_riscv64``:

.. code-block:: bash

   west build -b native_sim -t run samples/posix/hello_world

To build and run on real hardware, first build the application, then flash it:

.. code-block:: bash

   west build -b <your_board> samples/posix/hello_world
   west flash

Expected output:

.. code-block:: console

   *** Booting Zephyr OS build v4.3.0 ***
   Hello World! native_sim/native from pthread_t 0x8058400
   pthread_t returned 42

Hello, Zephyr world!
********************

Some Zephyr applications do not use automatic thread or thread-stack allocation
because it can be viewed as a form of dynamic memory allocation.  Dynamic memory
allocation is prohibited by some industry standards for safety reasons.
Supporting those applications with as few pain points as possible is of paramount
importance for both The Zephyr Project and Zephyr's POSIX reference
implementation.

Although the POSIX API does not directly support operations on pre-initialized
threads or threading primitives, ``posix-next`` has moved to a model where there
is a 1:1 correspondence between Zephyr and POSIX threading primitives:

- ``k_condvar`` <=> ``pthread_cond_t``
- ``k_mutex`` <=> ``pthread_mutex_t``
- ``k_thread`` <=> ``pthread_t``

With that, converting between Zephyr and POSIX threading primitives is
as simple as a pointer cast.

.. literalinclude:: ../../samples/posix/hello_zephyr/src/main.c
   :language: c

Build and run
=============

.. code-block:: bash

   west build -b native_sim -t run samples/posix/hello_zephyr

To build for real hardware, substitute the board name:

.. code-block:: bash

   west build -b <your_board> samples/posix/hello_zephyr
   west flash

Expected output:

.. code-block:: console

   *** Booting Zephyr OS build v4.3.0 ***
   Hello World! native_sim/native from k_thread 0x8057420
   k_thread returned 42

Build documentation locally
***************************

Building documentation for ``posix-next`` follows the same process as Zephyr.
Before continuing, please ensure that all
`Zephyr documentation build requirements
<https://docs.zephyrproject.org/latest/contribute/documentation/generation.html>`_
are met.

.. code-block:: bash

   cmake -GNinja -Bdoc/_build doc
   ninja -C doc/_build html
   python3 -m http.server -d doc/_build/html --bind 127.0.0.1
