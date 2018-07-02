cmake_minimum_required(VERSION 3.10)
project(mips_simulator)

set(CMAKE_CXX_STANDARD 17)

add_executable(mips_simulator main.cpp paser.h scanner.h MipsTest/codeline.h constant.h codeline.h)