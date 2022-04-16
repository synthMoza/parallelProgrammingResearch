cmake_minimum_required(VERSION 3.14)

project(transportEquationLab)

set(CMAKE_CXX_STANDARD 17)

# Set variables with all files
set(SOURCE_DIR source)
set(INCLUDE_DIR include)

set(SOURCE ${SOURCE_DIR}/Main.cpp ${SOURCE_DIR}/TransportEquationSolver.cpp)
set(HEADERS ${INCLUDE_DIR}/TransportEquationSolver.h)

# Find MPI on this computer
find_package(MPI REQUIRED)
include_directories(${PROJECT_NAME} ${INCLUDE_DIR} ${MPI_INCLUDE_PATH})

# Create executable and link lib
add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${MPI_CXX_LIBRARIES})