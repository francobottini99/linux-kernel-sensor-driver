cmake_minimum_required (VERSION 3.18)

project(SERVER)

file(MAKE_DIRECTORY bin)

include_directories(${CMAKE_SOURCE_DIR}/inc)
include_directories(${CMAKE_SOURCE_DIR}/src)

set(SOURCE src/server.c src/threads_handle.c src/utils.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pedantic -Wextra -Wconversion -std=gnu11")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(server ${SOURCE})

target_link_libraries(server pthread)
