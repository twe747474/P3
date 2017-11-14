cmake_minimum_required(VERSION 3.8)
project(P3)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp manager.h router.cpp router.h sharedFunctions.h sharedFunctions.cpp)
find_package (Threads)
add_executable(P3 ${SOURCE_FILES})