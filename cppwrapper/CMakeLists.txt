cmake_minimum_required(VERSION 3.14)

project(mpicppwrapper)

set(INCLUDE_DIR include)
set(SRC_DIR source)

set(SRC ${SRC_DIR}/main.cpp)
set(HEADERS ${INCLUDE_DIR}/core.h ${INCLUDE_DIR}/comm.h ${INCLUDE_DIR}/traits.h)

find_package(MPI REQUIRED)

include_directories(${PROJECT_NAME} ${MPI_INCLUDE_PATH} ${INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${SRC} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${MPI_C_LIBRARIES})