cmake_minimum_required(VERSION 3.16)
project(sum_numbers)

set(INCLUDE_DIR include)

set(SOURCE source/main.cpp source/input.cpp)
set(INCLUDE ${INCLUDE_DIR}/input.h)

add_executable(${PROJECT_NAME} ${INCLUDE} ${SOURCE})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

message(STATUS "Looking for OpenMP library...")
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "Can't find OpenMP library!")
endif()
message(STATUS "Done!")