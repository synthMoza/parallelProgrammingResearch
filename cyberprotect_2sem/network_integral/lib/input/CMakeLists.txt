project(input)

set(SOURCE_DIR src)
set(INCLUDE_DIR include)

set(SOURCE ${SOURCE_DIR}/input.c)
set(INCLUDE ${INCLUDE_DIR}/input.h)

add_library(${PROJECT_NAME} STATIC ${SOURCE} ${INCLUDE})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})
