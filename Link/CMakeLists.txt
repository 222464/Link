cmake_minimum_required(VERSION 3.0)

project(Link)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "-std=c++14 -O3")
endif()

FIND_PACKAGE ( Threads REQUIRED )

include_directories("${PROJECT_SOURCE_DIR}/source")

SET(DUKTAPE_SRC_DIR "${PROJECT_SOURCE_DIR}/source/duktape")

INCLUDE_DIRECTORIES(${DUKTAPE_SRC_DIR})

ADD_LIBRARY(
    duktape
    ${DUKTAPE_SRC_DIR}/duktape.c
)

# This is only required for the script to work in the version control
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")
 
find_package(SFML 2 REQUIRED system window graphics)
 
include_directories(${SFML_INCLUDE_DIR})

file(GLOB_RECURSE LINK_SRC
    "source/*.h"
    "source/*.cpp"
)
 
add_executable(Link ${LINK_SRC})

target_link_libraries(Link ${SFML_LIBRARIES})

target_link_libraries(Link duktape)

TARGET_LINK_LIBRARIES(Link ${CMAKE_THREAD_LIBS_INIT})
