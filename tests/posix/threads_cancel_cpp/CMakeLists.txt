cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(posix_threads_cancel_cpp)

target_compile_options(app PRIVATE
	-U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L
	-U_XOPEN_SOURCE -D_XOPEN_SOURCE=700
)

target_sources(app PRIVATE src/main.cpp)
