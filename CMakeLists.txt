cmake_minimum_required(VERSION 3.15.3)

project(huff)
set(CMAKE_C_FLAGS "-Wall -Werror -O3")
add_executable(huff huff.c)
add_executable(unhuff unhuff.c)