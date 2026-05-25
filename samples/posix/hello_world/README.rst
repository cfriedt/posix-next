.. zephyr:code-sample:: posix-hello-world
   :name: Hello, POSIX world!

   Create a POSIX thread with ``pthread_create()`` and join it with ``pthread_join()``.

Overview
********

This sample demonstrates portable POSIX thread usage on Zephyr. A worker thread is created with
`pthread_create()`_, joined with `pthread_join()`_, and its return value is printed. The sample
relies on automatic thread and thread-stack allocation so that no Zephyr-specific APIs appear in
application code.

Building and Running
********************

.. zephyr-app-commands::
   :zephyr-app: samples/posix/hello_world
   :host-os: unix
   :board: native_sim
   :goals: run
   :compact:

Sample Output
=============

.. code-block:: console

    Hello World! native_sim from pthread_t 0x7f...
    pthread_t returned 42

.. _pthread_create(): https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_create.html
.. _pthread_join(): https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_join.html
