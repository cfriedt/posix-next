.. zephyr:code-sample:: posix-hello-zephyr
   :name: Hello, Zephyr world!

   Define a native ``k_thread`` at compile time and join it via ``pthread_join()``.

Overview
********

This sample demonstrates the 1:1 mapping between Zephyr kernel threads and POSIX threads. A
``k_thread`` is statically defined with `K_THREAD_DEFINE()`_ (deferred start), then started at
runtime and joined using `pthread_join()`_. The thread's return value, set with
``k_thread_result_set()``, is retrieved through the standard POSIX interface.

This pattern is useful for safety-critical applications where dynamic memory allocation is
prohibited: the thread and its stack are allocated at compile time, yet the application can still
use the POSIX threading API for synchronization.

Building and Running
********************

.. zephyr-app-commands::
   :zephyr-app: samples/posix/hello_zephyr
   :host-os: unix
   :board: native_sim
   :goals: run
   :compact:

Sample Output
=============

.. code-block:: console

    Hello World! native_sim from k_thread 0x...
    k_thread returned 42

.. _K_THREAD_DEFINE(): https://docs.zephyrproject.org/latest/kernel/services/threads/index.html#c.K_THREAD_DEFINE
.. _pthread_join(): https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_join.html
