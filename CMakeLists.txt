cmake_minimum_required(VERSION 3.12)
project(uVM C)

set(CMAKE_C_STANDARD 11)

add_executable(uVM main.c vm.c vm.h)