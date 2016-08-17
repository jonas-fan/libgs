cmake_minimum_required(VERSION 2.8)

project(libgs C)

set(TARGET_NAME "gs")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wextra -pedantic -std=c99 -g -D_GNU_SOURCE")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)

add_subdirectory(src)
add_subdirectory(tests)