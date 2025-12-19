# SPDX-License-Identifier: Apache-2.0

if(CONFIG_TC_PROVIDES_EVENTFD)
  return()
endif()

zephyr_library()
zephyr_library_sources(eventfd.c)
