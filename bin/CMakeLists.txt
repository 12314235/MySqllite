cmake_minimum_required(VERSION 3.24)
project(labwork8-12314235_1)

set(CMAKE_CXX_STANDARD 20)

add_executable(labwork8-12314235_1 main.cpp MySQLite.cpp MySQLite.h)

add_library(lib MySQLite.h MySQLite.cpp)

