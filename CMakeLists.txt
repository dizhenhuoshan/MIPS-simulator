cmake_minimum_required(VERSION 3.10)
project(mips_simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_executable(mips_simulator main.cpp paser.h scanner.h constant.h memeory.h register.h decoder.h)