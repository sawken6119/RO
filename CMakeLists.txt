cmake_minimum_required(VERSION 3.30)
project(RO C)

set(CMAKE_C_STANDARD 11)

add_executable(RO main.c
        transport.c
        transport.h)
