cmake_minimum_required(VERSION 3.12)
project(uVM C)

set(CMAKE_C_STANDARD 11)

add_executable(uVM main.c vm.c vm.h asm/asm.c asm/asm.h asm/token.c asm/token.h)
target_include_directories(uVM PRIVATE asm)