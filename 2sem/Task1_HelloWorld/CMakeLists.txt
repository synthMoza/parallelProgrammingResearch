cmake_minimum_required(VERSION 3.14)
project(HelloWorld)

set(SOURCE_BASIC source/main_basic.cpp)
set(SOURCE_ADVANCED source/main_advanced.cpp)

add_executable(${PROJECT_NAME}_basic ${SOURCE_BASIC} ${INCLUDE})
add_executable(${PROJECT_NAME}_advanced ${SOURCE_ADVANCED} ${INCLUDE})

message(STATUS "Looking for OpenMP library...")
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME}_basic PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(${PROJECT_NAME}_advanced PUBLIC OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "Can't find OpenMP library!")
endif()
message(STATUS "Done!")