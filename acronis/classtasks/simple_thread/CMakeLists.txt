project(thread_race)
cmake_minimum_required(VERSION 3.14)

set(SOURCE source/main.cpp)

add_executable(${PROJECT_NAME} ${SOURCE})

find_package(Threads)
if (Threads_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
else()
    message(FATAL_ERROR "Couldn't find any thread lib!")
endif()