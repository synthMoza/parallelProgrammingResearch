cmake_minimum_required(VERSION 3.16)
project(prime_numbers)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SOURCE_DIR source)
set(INCLUDE_DIR include)

set(SOURCE ${SOURCE_DIR}/main.cpp ${SOURCE_DIR}/input.cpp)

# Main target executable
add_executable(${PROJECT_NAME} ${SOURCE})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

message(STATUS "Looking for OpenMP library...")
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "Can't find OpenMP library!")
endif()
