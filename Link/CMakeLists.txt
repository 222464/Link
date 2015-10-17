cmake_minimum_required(VERSION 3.0)

project(Link)

include_directories("${PROJECT_SOURCE_DIR}/source")

# This is only required for the script to work in the version control
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")
 
find_package(SFML 2 REQUIRED system window graphics)
 
include_directories(${SFML_INCLUDE_DIR})

file(GLOB_RECURSE LINK_SRC
    "source/*.h"
    "source/*.cpp"
)
 
add_executable(Link ${LINK_SRC})

target_link_libraries(Link ${SFML_LIBRARIES}})