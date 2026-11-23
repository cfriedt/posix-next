.. _posix_option_group_networking:

POSIX_NETWORKING
================

The function ``sockatmark()`` is not yet supported and is expected to fail setting ``errno``
to ``ENOSYS`` :ref:`†<posix_undefined_behaviour>`.

:c:func:`getaddrinfo` resolves an address literal itself, consults the hosts database next, and
hands a name it does not list to Zephyr's resolver (:kconfig:option:`CONFIG_DNS_RESOLVER`); a
service is a port number. See :ref:`posix_name_resolution`.

Enable this option group with :kconfig:option:`CONFIG_POSIX_NETWORKING`.

The host lookup of POSIX.1-2001, ``gethostbyname()``, ``gethostbyaddr()``, ``hstrerror()`` and
``h_errno``, which POSIX.1-2008 removed, is provided for the programs that still use it: a name
resolves through :c:func:`getaddrinfo`, an address through the hosts database, each into one static
result holding the first address. ``<netdb.h>`` declares them only for a program that targets
POSIX.1-2001 (Issue 6) or earlier, with ``_POSIX_C_SOURCE`` at most ``200112L`` and
``_XOPEN_SOURCE`` at most ``600``, or that asks for the extensions with ``_DEFAULT_SOURCE``,
``_BSD_SOURCE`` or ``_GNU_SOURCE``; a program built to POSIX.1-2008 does not see them.

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
    :c:func:`shutdown`,yes
    :c:func:`socket`,yes
    :c:func:`sockatmark`,yes :ref:`†<posix_undefined_behaviour>`
    :c:func:`socketpair`,yes

