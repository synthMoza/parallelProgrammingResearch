cmake_minimum_required(VERSION 3.16)
project(multiply_matrices)

set(SOURCE_DIR source)
set(INCLUDE_DIR include)

set(SOURCE ${SOURCE_DIR}/main.cpp ${SOURCE_DIR}/input.cpp)
set(SOURCE_BENCH ${SOURCE_DIR}/bench.cpp)

# Main target executable
add_executable(${PROJECT_NAME} ${SOURCE})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

# Set up benchmark target
if (DEFINED WITH_BENCHMARK)
    add_subdirectory(3rd_party/googletest) # required for google benchmark
    add_subdirectory(3rd_party/benchmark)
    add_executable(${PROJECT_NAME}_bench ${SOURCE_BENCH})

    target_link_libraries(${PROJECT_NAME}_bench PRIVATE benchmark::benchmark)
    target_include_directories(${PROJECT_NAME}_bench PRIVATE ${INCLUDE_DIR})
endif()

message(STATUS "Looking for OpenMP library...")
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
    if (DEFINED WITH_BENCHMARK)
        target_link_libraries(${PROJECT_NAME}_bench PUBLIC OpenMP::OpenMP_CXX)
    endif()
else()
    message(FATAL_ERROR "Can't find OpenMP library!")
endif()
