cmake_minimum_required(VERSION 3.19)

# ./tests/CMakeLists.txt
project(tests)

# tests in *_test.cpp files, each contains main function
file(GLOB TEST_SOURCES *_test.cpp)

# add executable for each test
# expecting return code 0
enable_testing()
message(STATUS "Adding tests:")
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    message(STATUS "Adding test: ${TEST_NAME}")
endforeach()
