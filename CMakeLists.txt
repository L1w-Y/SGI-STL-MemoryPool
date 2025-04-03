cmake_minimum_required(VERSION 3.30)
project(MemoryPool)

set(CMAKE_CXX_STANDARD 20)

add_executable(MemoryPool main.cpp
        MemoryPool.cpp
        MemoryPool.h)
