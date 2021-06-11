#template 
cmake_minimum_required(VERSION 3.10)
project(executable)

set(SOURCES "main.cc"
            "src/future_promise.cc")

set (HEADERS "include/future_promise.h"
             "include/is_valid.h")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O2")

find_package (Threads)
include_directories("include/")
add_executable(executable ${HEADERS} ${SOURCES})
target_link_libraries(executable Threads::Threads)
