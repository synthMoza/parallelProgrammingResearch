cmake_minimum_required(VERSION 3.22)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O2")

add_subdirectory(lib/input)
add_subdirectory(lib/integral)

project(input)

set(SOURCE_DIR src)
set(INCLUDE_DIR include)

set(INCLUDE ${INCLUDE_DIR}/network.h)

add_executable(client ${SOURCE_DIR}/client.c ${INCLUDE})
target_include_directories(client PRIVATE ${INCLUDE_DIR})
target_link_libraries(client PRIVATE input integral m)

add_executable(server ${SOURCE_DIR}/server.c ${INCLUDE})
target_include_directories(server PRIVATE ${INCLUDE_DIR})
target_link_libraries(server PRIVATE input integral)

add_custom_command(
        TARGET server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/tests/test.py
                ${CMAKE_CURRENT_BINARY_DIR}/test.py)
