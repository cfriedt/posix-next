.. _posix_aep:

POSIX Application Environment Profiles (AEP)
############################################

Although inactive, `IEEE 1003.13-2003`_ defined a number of AEP that inspired the modern
subprofiling options of `IEEE 1003.1-2017`_. The single-purpose realtime system profiles
are listed below, for reference, in terms that agree with the current POSIX-1 standard.

.. tabs::

   .. tab:: System Interfaces

      .. raw:: html

         <div class="figure align-center aep-diagram" data-aep-svg="si.svg"
              aria-label="Required System Interfaces">
           <p class="caption"><span class="caption-text">System Interfaces</span></p>
         </div>

      .. _posix_aep_system_interfaces:
      .. _system_interfaces:

      The required POSIX :ref:`System Interfaces<posix_system_interfaces_required>` are supported
      by each Application Environment Profile.

      .. csv-table:: System Interfaces
         :header: Symbol, Support, Remarks
         :widths: 50, 10, 50

          _POSIX_VERSION, 200809L, :kconfig:option:`CONFIG_POSIX_SYSTEM_INTERFACES`

      .. csv-table:: Required POSIX System Interfaces
         :header: Symbol, Support, Remarks
         :widths: 50, 10, 50

          :ref:`_POSIX_ASYNCHRONOUS_IO<posix_option_asynchronous_io>`, 200809L, :kconfig:option:`CONFIG_POSIX_ASYNCHRONOUS_IO` :ref:`†<posix_undefined_behaviour>`
          :ref:`_POSIX_BARRIERS<posix_option_barriers>`, 200809L, :kconfig:option:`CONFIG_POSIX_BARRIERS`
          :ref:`_POSIX_CLOCK_SELECTION<posix_option_clock_selection>`, 200809L, :kconfig:option:`CONFIG_POSIX_CLOCK_SELECTION`
          :ref:`_POSIX_MAPPED_FILES<posix_option_mapped_files>`, 200809L, :kconfig:option:`CONFIG_POSIX_MAPPED_FILES`
          :ref:`_POSIX_MEMORY_PROTECTION<posix_option_memory_protection>`, 200809L, :kconfig:option:`CONFIG_POSIX_MEMORY_PROTECTION` :ref:`†<posix_undefined_behaviour>`
          :ref:`_POSIX_READER_WRITER_LOCKS<posix_option_reader_writer_locks>`, 200809L, :kconfig:option:`CONFIG_POSIX_RW_LOCKS`
          :ref:`_POSIX_REALTIME_SIGNALS<posix_option_realtime_signals>`, 200809L, :kconfig:option:`CONFIG_POSIX_REALTIME_SIGNALS`
          :ref:`_POSIX_SEMAPHORES<posix_option_semaphores>`, 200809L, :kconfig:option:`CONFIG_POSIX_SEMAPHORES`
          :ref:`_POSIX_SPIN_LOCKS<posix_option_spin_locks>`, 200809L, :kconfig:option:`CONFIG_POSIX_SPIN_LOCKS`
          :ref:`_POSIX_THREAD_SAFE_FUNCTIONS<posix_option_thread_safe_functions>`, 200809L, :kconfig:option:`CONFIG_POSIX_C_LANG_SUPPORT_R` and :kconfig:option:`CONFIG_POSIX_FILE_SYSTEM_R` and :kconfig:option:`CONFIG_POSIX_FILE_LOCKING`
          :ref:`_POSIX_THREADS<posix_option_threads>`, 200809L, :kconfig:option:`CONFIG_POSIX_THREADS`
          :ref:`_POSIX_TIMEOUTS<posix_option_timeouts>`, 200809L, :kconfig:option:`CONFIG_POSIX_TIMERS`
          :ref:`_POSIX_TIMERS<posix_option_timers>`, 200809L, :kconfig:option:`CONFIG_POSIX_TIMERS`

   .. tab:: PSE51

      .. raw:: html

         <div class="figure align-center aep-diagram" data-aep-svg="aep-pse51.svg"
              aria-label="Minimal Realtime System Profile (PSE51)">
           <p class="caption"><span class="caption-text">Minimal Realtime System Profile (PSE51)</span></p>
         </div>

      .. _posix_aep_pse51:
      .. _minimal_realtime_system_profile_pse51:

      The *Minimal Realtime System Profile* (PSE51) includes all of the
      :ref:`System Interfaces<posix_system_interfaces_required>` along with several additional
      features.

      .. Conforming implementations shall define _POSIX_AEP_REALTIME_MINIMAL to the value 200312L

      .. csv-table:: PSE51 System Interfaces
         :header: Symbol, Support, Remarks
         :widths: 50, 10, 50

          _POSIX_AEP_REALTIME_MINIMAL, -1, :kconfig:option:`CONFIG_POSIX_AEP_REALTIME_MINIMAL`

      .. csv-table:: PSE51 Option Groups
         :header: Symbol, Support, Remarks
         :widths: 50, 10, 50

          :ref:`POSIX_DEVICE_IO <posix_option_group_device_io>`, yes, :kconfig:option:`CONFIG_POSIX_DEVICE_IO`
          :ref:`POSIX_FILE_LOCKING <posix_option_group_file_locking>`, yes, :kconfig:option:`CONFIG_POSIX_FILE_LOCKING`
          :ref:`POSIX_SIGNALS <posix_option_group_signals>`,, :kconfig:option:`CONFIG_POSIX_SIGNALS` :ref:`†<posix_undefined_behaviour>`
          :ref:`POSIX_SINGLE_PROCESS <posix_option_group_single_process>`, yes, :kconfig:option:`CONFIG_POSIX_SINGLE_PROCESS`
          :ref:`POSIX_THREADS_EXT <posix_option_group_posix_threads_ext>`, yes, :kconfig:option:`CONFIG_POSIX_THREADS_EXT`

      .. csv-table:: PSE51 Option Requirements
         :header: Symbol, Support, Remarks
         :widths: 50, 10, 50

          :ref:`_POSIX_FSYNC <posix_option_fsync>`, 200809L, :kconfig:option:`CONFIG_POSIX_FSYNC`
          :ref:`_POSIX_MEMLOCK <posix_option_memlock>`, 200809L, :kconfig:option:`CONFIG_POSIX_MEMLOCK` :ref:`†<posix_undefined_behaviour>`
          :ref:`_POSIX_MEMLOCK_RANGE <posix_option_memlock_range>`, 200809L, :kconfig:option:`CONFIG_POSIX_MEMLOCK_RANGE`
          :ref:`_POSIX_SHARED_MEMORY_OBJECTS <posix_option_shared_memory_objects>`, 200809L, :kconfig:option:`CONFIG_POSIX_SHARED_MEMORY_OBJECTS`
          :ref:`_POSIX_SYNCHRONIZED_IO <posix_option_synchronized_io>`, 200809L, :kconfig:option:`CONFIG_POSIX_SYNCHRONIZED_IO`
          :ref:`_POSIX_THREAD_ATTR_STACKADDR<posix_option_thread_attr_stackaddr>`, 200809L, :kconfig:option:`CONFIG_POSIX_THREAD_ATTR_STACKADDR`
          :ref:`_POSIX_THREAD_ATTR_STACKSIZE<posix_option_thread_attr_stacksize>`, 200809L, :kconfig:option:`CONFIG_POSIX_THREAD_ATTR_STACKSIZE`
          :ref:`_POSIX_THREAD_CPUTIME <posix_option_thread_cputime>`, 200809L, :kconfig:option:`CONFIG_POSIX_THREAD_CPUTIME`
          :ref:`_POSIX_THREAD_PRIO_INHERIT <posix_option_thread_prio_inherit>`, 200809L, :kconfig:option:`CONFIG_POSIX_THREAD_PRIO_INHERIT`
          :ref:`_POSIX_THREAD_PRIO_PROTECT <posix_option_thread_prio_protect>`, 200809L, :kconfig:option:`CONFIG_POSIX_THREAD_PRIO_PROTECT`
          :ref:`_POSIX_THREAD_PRIORITY_SCHEDULING <posix_option_thread_priority_scheduling>`, 200809L, :kconfig:option:`CONFIG_POSIX_THREAD_PRIORITY_SCHEDULING`
          _POSIX_THREAD_SPORADIC_SERVER, -1,

   .. tab:: PSE52

      .. raw:: html

         <div class="figure align-center aep-diagram" data-aep-svg="aep-pse52.svg"
              aria-label="Realtime Controller System Profile (PSE52)">
           <p class="caption"><span class="caption-text">Realtime Controller System Profile (PSE52)</span></p>
         </div>

      .. _posix_aep_pse52:
      .. _realtime_controller_system_profile_pse52:

      The *Realtime Controller System Profile* (PSE52) includes all features from PSE51 and the
      :ref:`System Interfaces<posix_system_interfaces_required>`.

      .. Conforming implementations shall define _POSIX_AEP_REALTIME_CONTROLLER to the value 200312L

      .. csv-table:: PSE52 System Interfaces
         :header: Symbol, Support, Remarks
         :widths: 50, 10, 50

          _POSIX_AEP_REALTIME_CONTROLLER, -1, :kconfig:option:`CONFIG_POSIX_AEP_REALTIME_CONTROLLER`

      .. csv-table:: PSE52 Option Groups
         :header: Symbol, Support, Remarks
         :widths: 50, 10, 50

          :ref:`POSIX_C_LANG_MATH <posix_option_group_c_lang_math>`, yes,
          :ref:`POSIX_FD_MGMT <posix_option_group_fd_mgmt>`,, :kconfig:option:`CONFIG_POSIX_FD_MGMT`
          :ref:`POSIX_FILE_SYSTEM <posix_option_group_file_system>`,, :kconfig:option:`CONFIG_POSIX_FILE_SYSTEM`

      .. csv-table:: PSE52 Option Requirements
         :header: Symbol, Support, Remarks
         :widths: 50, 10, 50

          :ref:`_POSIX_MESSAGE_PASSING <posix_option_message_passing>`, 200809L, :kconfig:option:`CONFIG_POSIX_MESSAGE_PASSING`

      .. note::
         When using Newlib, Picolibc, or other C libraries conforming to the ISO C Standard, the
         ``POSIX_C_LANG_MATH`` Option Group is considered supported since C11.

      .. note::
        POSIX tracing appeared in IEEE 1003.13 but was removed from the specification in Issue 8
        and is not listed here as a PSE52 option requirement :ref:`†<posix_undefined_behaviour>`.

   .. tab:: PSE53

      .. raw:: html

         <div class="figure align-center aep-diagram" data-aep-svg="aep-pse53.svg"
              aria-label="Dedicated Realtime System Profile (PSE53)">
           <p class="caption"><span class="caption-text">Dedicated Realtime System Profile (PSE53)</span></p>
         </div>

      .. _posix_aep_pse53:
      .. _dedicated_realtime_system_profile_pse53:

      The *Dedicated Realtime System Profile* (PSE53) includes all features from PSE52, PSE51, and
      the :ref:`System Interfaces<posix_system_interfaces_required>`.

      .. Conforming implementations shall define _POSIX_AEP_REALTIME_DEDICATED to the value 200312L

      .. csv-table:: PSE53 System Interfaces
         :header: Symbol, Support, Remarks
         :widths: 50, 10, 50

          _POSIX_AEP_REALTIME_DEDICATED, -1, :kconfig:option:`CONFIG_POSIX_AEP_REALTIME_DEDICATED`

      .. csv-table:: PSE53 Option Groups
         :header: Symbol, Support, Remarks
         :widths: 50, 10, 50

          :ref:`POSIX_MULTI_PROCESS <posix_option_group_multi_process>`,, :kconfig:option:`CONFIG_POSIX_MULTI_PROCESS` :ref:`†<posix_undefined_behaviour>`
          :ref:`POSIX_NETWORKING <posix_option_group_networking>`,, :kconfig:option:`CONFIG_POSIX_NETWORKING`
          :ref:`POSIX_PIPE <posix_option_group_pipe>`,,
          :ref:`POSIX_SIGNAL_JUMP <posix_option_group_signal_jump>`,,

      .. csv-table:: PSE53 Option Requirements
         :header: Symbol, Support, Remarks
         :widths: 50, 10, 50

          :ref:`_POSIX_CPUTIME <posix_option_cputime>`, 200809L, :kconfig:option:`CONFIG_POSIX_CPUTIME`
          _POSIX_PRIORITIZED_IO, -1,
          :ref:`_POSIX_PRIORITY_SCHEDULING <posix_option_priority_scheduling>`, 200809L, :kconfig:option:`CONFIG_POSIX_PRIORITY_SCHEDULING`
          :ref:`_POSIX_RAW_SOCKETS <posix_option_raw_sockets>`, 200809L, :kconfig:option:`CONFIG_POSIX_RAW_SOCKETS`
          _POSIX_SPAWN, -1, :ref:`†<posix_undefined_behaviour>`
          _POSIX_SPORADIC_SERVER, -1, :ref:`†<posix_undefined_behaviour>`


.. note::
   In the *Option Groups* tables above, a blank Support cell with a Kconfig remark means the
   profile enables that :ref:`option group <posix_option_groups>` but it is not fully
   implemented. ``yes`` means the group is considered fully supported. See each group page
   for per-API detail.

.. note::
   PSE54 is not considered at this time.

.. _IEEE 1003.1-2017: https://standards.ieee.org/ieee/1003.1/7101/
.. _IEEE 1003.13-2003: https://standards.ieee.org/ieee/1003.13/3322/
