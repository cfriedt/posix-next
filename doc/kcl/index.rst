.. _kcl:

Kernel Compatibility Layers
###########################

A kernel compatibility layer provides another kernel's system call interface,
so that programs built for that kernel run on Zephyr unchanged. Layers are
enabled with :kconfig:option:`CONFIG_KCL` and live under :file:`subsys/kcl/` of the module.

.. toctree::
   :maxdepth: 1

   linux.rst
   ../samples/subsys/kcl/kcl
