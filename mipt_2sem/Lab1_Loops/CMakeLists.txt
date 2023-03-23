project(mpi_loops)
cmake_minimum_required(VERSION 3.14)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SRC_DIR source)
set(INCLUDE_DIR include)

find_package(MPI REQUIRED)

# 1a) Общая последовательная программа
add_executable(generic_sequential ${SRC_DIR}/generic_sequential.cpp)
target_link_libraries(generic_sequential PRIVATE m)

# 1b) Общая распараллеленная программа
add_executable(generic_parallel ${SRC_DIR}/generic_parallel.cpp)
target_link_libraries(generic_parallel PRIVATE m)
target_link_libraries(generic_parallel PRIVATE MPI::MPI_CXX)

# 2a) Персональная последовательная программа
add_executable(personal_sequential ${SRC_DIR}/personal_sequential.cpp)
target_link_libraries(personal_sequential PRIVATE m)

# 1b) Персональная распараллеленная программа
add_executable(personal_parallel ${SRC_DIR}/personal_parallel.cpp)
target_link_libraries(personal_parallel PRIVATE m)
target_link_libraries(personal_parallel PRIVATE MPI::MPI_CXX)


if (DEFINED OUTPUT_TO_FILE)
  add_compile_definitions(OUTPUT_TO_FILE)
endif()
