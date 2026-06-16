# Copyright (c) 2024 Tenstorrent
#
# SPDX-License-Identifier: Apache-2.0

config POSIX_API
	bool "POSIX APIs"
	imply POSIX_SYSTEM_INTERFACES
	imply POSIX_BASE_DEFINITIONS # clock_gettime(), pthread_create(), sem_get(), etc
	imply POSIX_AEP_REALTIME_MINIMAL # CLOCK_MONOTONIC, pthread_attr_setstack(), etc
	imply POSIX_NETWORKING if NETWORKING # inet_ntoa(), socket(), etc
	imply EVENTFD # eventfd(), eventfd_read(), eventfd_write()
	imply POSIX_FD_MGMT # open(), close(), read(), write()
	imply POSIX_MULTI_PROCESS # sleep(), getpid(), etc
	imply XSI_SINGLE_PROCESS # gettimeofday()
	select DEPRECATED
	help
	  This option is deprecated. Applications should select CONFIG_POSIX_AEP_CHOICE_BASE,
	  CONFIG_POSIX_AEP_CHOICE_PSE51, CONFIG_POSIX_AEP_CHOICE_PSE52, or
	  CONFIG_POSIX_AEP_CHOICE_PSE53. Libraries should depend on
	  CONFIG_POSIX_SYSTEM_INTERFACES and other POSIX Option Groups.

choice POSIX_AEP_CHOICE
	prompt "POSIX Subprofile"
	default POSIX_AEP_CHOICE_ZEPHYR
	help
	  This choice is intended to help users select the correct POSIX profile for their
	  application. Choices are based on IEEE 1003.13-2003 (now inactive / reserved) and
	  extrapolated to the more recent Subprofiling Option Groups in IEEE 1003.3-2017.

	  A subprofile enables its Option Groups and Options by default; each of them may
	  still be disabled individually, and further Option Groups may be enabled on top.

config POSIX_AEP_CHOICE_NONE
	bool "No POSIX subprofile"
	help
	  No POSIX subprofile is selected.

config POSIX_AEP_CHOICE_ZEPHYR
	bool "Minimal Zephyr System Profile"
	imply POSIX_C_LIB_EXT
	imply POSIX_C_LANG_SUPPORT_R
	help
	  Zephyr expects certain POSIX functions to be available throughout the build environment,
	  such as gmtime_r(), strnlen(), strtok_r(), and possibly others.

	  These functions are divided into two standalone Option Groups that may be enabled
	  independently of the remainder of the POSIX API implementation; namely POSIX_C_LIB_EXT and
	  POSIX_C_LANG_SUPPORT_R. If not referenced by the Zephyr kernel or application, there are no
	  resource implications for enabling these option groups.

	  Unlike pre-defined, standard POSIX subprofiles, this subprofile is custom to Zephyr and
	  therefore does not need to include the base definitions or system interfaces that would
	  otherwise be required for a conformant POSIX system or subprofile. This system profile
	  does not itself meet the requirements for POSIX implementation conformance.

config POSIX_AEP_CHOICE_BASE
	bool "Minimal POSIX System Profile"
	imply POSIX_SYSTEM_INTERFACES
	help
	  Only enable the base definitions required for all POSIX systems.

config POSIX_AEP_CHOICE_PSE51
	bool "Minimal Realtime System Profile (PSE51)"
	imply POSIX_SYSTEM_INTERFACES
	imply POSIX_AEP_REALTIME_MINIMAL
	help
	  PSE51 includes POSIX Base Definitions (System Interfaces) plus PSE51 delta:
	  option groups POSIX_DEVICE_IO, POSIX_FILE_LOCKING, POSIX_SIGNALS,
	  POSIX_SINGLE_PROCESS, POSIX_THREADS_EXT; and options
	  _POSIX_FSYNC, _POSIX_MEMLOCK, _POSIX_MEMLOCK_RANGE,
	  _POSIX_SHARED_MEMORY_OBJECTS, _POSIX_SYNCHRONIZED_IO,
	  _POSIX_THREAD_ATTR_STACKADDR, _POSIX_THREAD_ATTR_STACKSIZE,
	  _POSIX_THREAD_CPUTIME, _POSIX_THREAD_PRIO_INHERIT,
	  _POSIX_THREAD_PRIO_PROTECT, _POSIX_THREAD_PRIORITY_SCHEDULING.

config POSIX_AEP_CHOICE_PSE52
	bool "Realtime Controller System Profile (PSE52)"
	imply POSIX_SYSTEM_INTERFACES
	imply POSIX_AEP_REALTIME_MINIMAL
	imply POSIX_AEP_REALTIME_CONTROLLER
	imply POSIX_NON_PORTABLE
	help
	  PSE52 includes POSIX Base Definitions (System Interfaces) and all of PSE51.
	  Additionally enables POSIX_FD_MGMT, POSIX_FILE_SYSTEM, and
	  _POSIX_MESSAGE_PASSING.

config POSIX_AEP_CHOICE_PSE53
	bool "Dedicated Realtime System Profile (PSE53)"
	imply POSIX_SYSTEM_INTERFACES
	imply POSIX_AEP_REALTIME_MINIMAL
	imply POSIX_AEP_REALTIME_CONTROLLER
	imply POSIX_AEP_REALTIME_DEDICATED
	help
	  PSE53 includes POSIX Base Definitions, PSE51, and PSE52, plus PSE53 delta:
	  option groups POSIX_MULTI_PROCESS and POSIX_NETWORKING; and options
	  _POSIX_CPUTIME, _POSIX_PRIORITY_SCHEDULING, and _POSIX_RAW_SOCKETS.

config POSIX_AEP_CHOICE_NETAPP
	bool "Network Appliance Profile"
	imply POSIX_SYSTEM_INTERFACES
	imply POSIX_AEP_REALTIME_MINIMAL
	imply POSIX_AEP_REALTIME_CONTROLLER
	imply POSIX_NETWORKING if NETWORKING
	imply POSIX_RAW_SOCKETS if NETWORKING
	imply POSIX_CPUTIME
	imply POSIX_PRIORITY_SCHEDULING
	help
	  This profile is custom to Zephyr and does not correspond to a standard POSIX subprofile.
	  It includes all features of PSE52 as well as the networking interfaces of PSE53, but not
	  its multi-process, pipe, spawn or sporadic server interfaces. It is intended for
	  networked applications that do not need process management. The networking interfaces
	  are only enabled when CONFIG_NETWORKING is enabled.

# TODO: PSE54: Multi-purpose Realtime System Profile

endchoice # POSIX_AEP_CHOICE

config POSIX_TEST_LINUX_COMPAT
	bool "Test POSIX suites are compatible with Linux"
	depends on TEST
	help
	  Used by Twister linux_compat variants on native_sim. When set,
	  NATIVE_LIBC_INCOMPATIBLE is not selected, allowing host libc
	  (CONFIG_NATIVE_LIBC). Per-suite test Kconfig sets
	  CONFIG_TC_PROVIDES_<OPTION_GROUP>=y for the option group under test.

	  The "DUT" in this case becomes the testsuite itself. By transitive
	  property, if Linux passes against the testsuite and Zephyr passes
	  the testsuite, Linux and Zephyr are approximately compatible to the
	  extent of coverage provided by the testsuite.

if POSIX_SYSTEM_INTERFACES

# Note: mandatory options have changed significantly between Issues 6, 7, and 8
# https://pubs.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap02.html#tag_02_01_03_01
# https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap02.html#tag_02_01_03_01
# https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/V1_chap02.html#tag_02_01_03_01

# Mandatory POSIX System Interfaces (base profile)
config POSIX_BASE_DEFINITIONS
	bool
	default y
	imply POSIX_ASYNCHRONOUS_IO
	imply POSIX_BARRIERS
	imply POSIX_CLOCK_SELECTION
	imply POSIX_MAPPED_FILES
	imply POSIX_MEMORY_PROTECTION
	imply POSIX_MONOTONIC_CLOCK
	imply POSIX_RW_LOCKS
	imply POSIX_REALTIME_SIGNALS
	imply POSIX_SEMAPHORES
	imply POSIX_SPIN_LOCKS
	# see below to satisfy _POSIX_THREAD_SAFE_FUNCTIONS
	imply POSIX_THREADS
	imply POSIX_TIMEOUTS
	imply POSIX_TIMERS
	# to satisfy _POSIX_THREAD_SAFE_FUNCTIONS	
	imply POSIX_FILE_SYSTEM_R
	imply POSIX_C_LANG_SUPPORT_R
	help
	  This option is not user configurable. It may be configured indirectly by selecting
	  CONFIG_POSIX_AEP_CHOICE_BASE=y.

# This profile builds on top of POSIX_BASE_DEFINITIONS
config POSIX_AEP_REALTIME_MINIMAL
	bool
	# Option Groups
	imply POSIX_DEVICE_IO
	imply POSIX_FILE_LOCKING
	imply POSIX_SIGNALS
	imply POSIX_SINGLE_PROCESS
	imply POSIX_THREADS_EXT
	# Options
	imply POSIX_FSYNC
	imply POSIX_MEMLOCK
	imply POSIX_MEMLOCK_RANGE
	imply POSIX_SHARED_MEMORY_OBJECTS
	imply POSIX_SYNCHRONIZED_IO
	imply POSIX_THREAD_ATTR_STACKADDR
	imply POSIX_THREAD_ATTR_STACKSIZE
	imply POSIX_THREAD_CPUTIME
	imply POSIX_THREAD_PRIO_INHERIT
	imply POSIX_THREAD_PRIO_PROTECT
	imply POSIX_THREAD_PRIORITY_SCHEDULING
	# select POSIX_THREAD_SPORADIC_SERVER
	help
	  Internal PSE51 profile (POSIX_AEP_CHOICE_PSE51). Inherits
	  POSIX_BASE_DEFINITIONS and enables the PSE51 option groups and individual
	  options listed in the choice help for POSIX_AEP_CHOICE_PSE51.

# This profile builds on top of POSIX_AEP_REALTIME_MINIMAL
config POSIX_AEP_REALTIME_CONTROLLER
	bool
	# Option Groups
	imply POSIX_FD_MGMT
	imply POSIX_FILE_SYSTEM
	# Options
	imply POSIX_MESSAGE_PASSING
	help
	  Internal PSE52 profile (POSIX_AEP_CHOICE_PSE52). Inherits PSE51 and adds
	  POSIX_FD_MGMT, POSIX_FILE_SYSTEM, and _POSIX_MESSAGE_PASSING.

# This profile builds on top of POSIX_AEP_REALTIME_CONTROLLER
config POSIX_AEP_REALTIME_DEDICATED
	bool
	# Option Groups
	# POSIX_EVENT_MGMT (select, FD_SET, etc, moved to POSIX_DEVICE_IO)
	imply POSIX_MULTI_PROCESS
	imply POSIX_NETWORKING
	# select POSIX_PIPE
	# select POSIX_SIGNAL_JUMP
	# Options
	imply POSIX_CPUTIME
	# select POSIX_PRIORITIZED_IO
	imply POSIX_PRIORITY_SCHEDULING
	imply POSIX_RAW_SOCKETS
	# select POSIX_SPAWN
	# select POSIX_SPORADIC_SERVER
	help
	  Internal PSE53 profile (POSIX_AEP_CHOICE_PSE53). Inherits PSE52 and adds
	  POSIX_MULTI_PROCESS, POSIX_NETWORKING, _POSIX_CPUTIME,
	  _POSIX_PRIORITY_SCHEDULING, and _POSIX_RAW_SOCKETS.

endif # POSIX_SYSTEM_INTERFACES
