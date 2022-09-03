project(parallelSort)
cmake_minimum_required(VERSION 3.14)

# Convinient standard
set(CMAKE_C_STANDARD 99)

# Configure thread lib
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Set variables with all files
set(SOURCE_DIR source)
set(INCLUDE_DIR include)

set(SOURCE ${SOURCE_DIR}/parallel_sort.c ${SOURCE_DIR}/main.c ${SOURCE_DIR}/input.c)
set(HEADERS ${INCLUDE_DIR}/parallel_sort.h ${INCLUDE_DIR}/input.h)

# Create executable and link libraries
add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads) # Posix threads
target_link_libraries(${PROJECT_NAME} PRIVATE m)                # Math

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})