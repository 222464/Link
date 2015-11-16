cmake_minimum_required(VERSION 2.8)

project(Link)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "-std=c++14 -O3")
endif()

find_package(Threads REQUIRED)

# This is only required for the script to work in the version control
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")
 
find_package(SFML 2 REQUIRED system window graphics)
 
include_directories(${SFML_INCLUDE_DIR})

include_directories("${PROJECT_SOURCE_DIR}/source")

set(DUKTAPE_SRC_DIR "${PROJECT_SOURCE_DIR}/source/duktape")

include_directories(${DUKTAPE_SRC_DIR})

add_library(
    duktape
    ${DUKTAPE_SRC_DIR}/duktape.c
)

file(GLOB_RECURSE LINK_SRC
    "source/*.h"
    "source/*.cpp"
)

foreach(item ${LINK_SRC})
  if(${item} MATCHES "source/([a-z,A-Z,0-9,_]*).cpp")
    list(REMOVE_ITEM LINK_SRC ${item})
  endif()
endforeach(item)

add_library(
    learning_algorithms
    ${LINK_SRC}
)

add_executable(experiment-textprediction "source/TextPrediction.cpp")
target_link_libraries(experiment-textprediction ${CMAKE_THREAD_LIBS_INIT} learning_algorithms ${SFML_LIBRARIES} duktape)

add_executable(experiment-timeseries "source/TimeSeries.cpp")
target_link_libraries(experiment-timeseries ${CMAKE_THREAD_LIBS_INIT} learning_algorithms ${SFML_LIBRARIES} duktape)

add_executable(experiment-mnistvideo "source/MNIST_Video.cpp")
target_link_libraries(experiment-mnistvideo ${CMAKE_THREAD_LIBS_INIT} learning_algorithms ${SFML_LIBRARIES} duktape)
