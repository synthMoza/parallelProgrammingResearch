project(integralCalculation)
cmake_minimum_required(VERSION 3.14)

# Convinient standard
set(CMAKE_C_STANDARD 99)

# Configure thread lib
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Set variables with all files
set(SOURCE_DIR source)
set(INCLUDE_DIR include)

set(SOURCE ${SOURCE_DIR}/input.c ${SOURCE_DIR}/integral.c ${SOURCE_DIR}/main.c)
set(HEADERS ${INCLUDE_DIR}/input.h ${INCLUDE_DIR}/integral.h)

# Find MPI on this computer
find_package(MPI REQUIRED)
include_directories(${PROJECT_NAME} ${INCLUDE_DIR} ${MPI_INCLUDE_PATH})

# Create executable and link libraries
add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE MPI::MPI_C)       # MPI
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads) # Posix threads
target_link_libraries(${PROJECT_NAME} PRIVATE m)                # Math