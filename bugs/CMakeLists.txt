cmake_minimum_required(VERSION 3.19)
project(researh)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 20)

set (RESEARCH_CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set (RESEARCH_CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR})

set (RESEARCH_CMAKE_VERSION_MAJOR 0)
set (RESEARCH_CMAKE_VERSION_MINOR 1)
set (RESEARCH_CMAKE_VERSION_PATCH 0)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)

set (RESEARCH_SOURCES
        src/main.cpp
        src/base64.cpp
        src/maze_utils.cpp
)

# для Release сборки добавить все возможные оптимизации
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mtune=native")
endif()

# для всех сборок убрать лиммит на размер стека
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--stack,100000000")

add_executable(research ${RESEARCH_SOURCES})

target_include_directories(research PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/src"
)

add_subdirectory(tests)

enable_testing()



