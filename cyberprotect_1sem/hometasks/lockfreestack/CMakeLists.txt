project(lock_free_stack)
cmake_minimum_required(VERSION 3.22)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror ")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SOURCE_DIR source)
set(INCLUDE_DIR include)

find_package(Threads REQUIRED)

# Main target for local testing
set(MAIN_SOURCE ${SOURCE_DIR}/main.cpp)
add_executable(${PROJECT_NAME} ${MAIN_SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)

# Benchmark tests
# set(BENCH_SOURCE ${SOURCE_DIR}/bench.cpp)
# add_executable(${PROJECT_NAME}_bench ${BENCH_SOURCE})
# target_include_directories(${PROJECT_NAME}_bench PUBLIC ${INCLUDE_DIR})