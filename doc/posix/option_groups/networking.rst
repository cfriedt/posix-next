.. _posix_option_group_networking:

POSIX_NETWORKING
================

Enable this option group with :kconfig:option:`CONFIG_POSIX_NETWORKING`.

.. csv-table:: POSIX_NETWORKING
   :header: API, Supported
   :widths: 50,10

    :c:func:`accept`,yes
    :c:func:`bind`,yes
    :c:func:`connect`,yes
    :c:func:`endhostent`,yes
    :c:func:`endnetent`,yes
    :c:func:`endprotoent`,yes
    :c:func:`endservent`,yes
    :c:func:`freeaddrinfo`,yes
    :c:func:`gai_strerror`,yes
    :c:func:`getaddrinfo`,yes
    :c:func:`gethostent`,yes
    :c:func:`gethostname`,yes
    :c:func:`getnameinfo`,yes
    :c:func:`getnetbyaddr`,yes
    :c:func:`getnetbyname`,yes
    :c:func:`getnetent`,yes
    :c:func:`getpeername`,yes
    :c:func:`getprotobyname`,yes
    :c:func:`getprotobynumber`,yes
    :c:func:`getprotoent`,yes
    :c:func:`getservbyname`,yes
    :c:func:`getservbyport`,yes
    :c:func:`getservent`,yes
    :c:func:`getsockname`,yes
    :c:func:`getsockopt`,yes
    :c:func:`htonl`,yes
    :c:func:`htons`,yes
    :c:func:`if_freenameindex`,yes
    :c:func:`if_indextoname`,yes
    :c:func:`if_nameindex`,yes
    :c:func:`if_nametoindex`,yes
    :c:func:`inet_addr`,yes
    :c:func:`inet_ntoa`,yes
    :c:func:`inet_ntop`,yes
    :c:func:`inet_pton`,yes
    :c:func:`listen`,yes
    :c:func:`ntohl`,yes
    :c:func:`ntohs`,yes
    :c:func:`recv`,yes
    :c:func:`recvfrom`,yes
    :c:func:`recvmsg`,yes
    :c:func:`send`,yes
    :c:func:`sendmsg`,yes
    :c:func:`sendto`,yes
    :c:func:`sethostent`,yes
    :c:func:`setnetent`,yes
    :c:func:`setprotoent`,yes
    :c:func:`setservent`,yes
    :c:func:`setsockopt`,yes
    :c:func:`shutdown`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`socket`,yes
    :c:func:`sockatmark`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`socketpair`,yes

.. note:: The function ``sockatmark()`` is not yet supported and is expected to fail setting ``errno``
   to ``ENOSYS`` :ref:`†<posix_undefined_behaviour>`.

.. note:: ``shutdown()`` with ``SHUT_WR`` or ``SHUT_RDWR`` is not supported by the Zephyr socket stack;
   those calls fail with ``errno`` set to ``ENOTSUP``.
