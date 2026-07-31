.. zephyr:code-sample:: posix-message-passing
   :name: Message Queues

   Exchange prioritised messages and receive arrival notifications with POSIX message queues.

Overview
********

This sample walks through the POSIX message queue API:

* a named queue is created with :c:func:`mq_open` and persists until :c:func:`mq_unlink`,
* messages are sent at different priorities with :c:func:`mq_send` and received in
  descending priority order (FIFO among equal priorities) with :c:func:`mq_receive`,
  which also reports each message's actual length and priority,
* :c:func:`mq_notify` registers a ``SIGEV_THREAD`` notification that runs a function in a
  new thread when a message arrives at the empty queue, demonstrating the classic
  re-register-then-drain idiom (a registration is consumed when it fires, and arrivals at
  a non-empty queue do not fire),
* :c:func:`mq_notify` then registers a ``SIGEV_SIGNAL`` notification, and the arrival
  signal - carrying the registered value, with ``si_code`` ``SI_MESGQ`` - is accepted
  synchronously with :c:func:`sigtimedwait`,
* :c:func:`mq_getattr` reports the queue geometry and fill level.

Building and Running
********************

This project outputs to the console. It can be built and executed on QEMU as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/posix/message_passing
   :host-os: unix
   :board: qemu_x86
   :goals: run
   :compact:

Sample Output
=============

.. code-block:: console

   received [prio 6]: urgent: fire drill
   received [prio 3]: reminder: standup at 10
   received [prio 3]: reminder: lunch at noon
   received [prio 0]: newsletter
   notified: 1 message(s) pending
   drained [prio 1]: notify works
   notified: 1 message(s) pending
   drained [prio 1]: and re-arms, too
   queue geometry: 8 x 48 bytes
   done
