cmake_minimum_required(VERSION 3.21.2)

project(ECU_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(test
    test.cc
    ${CMAKE_SOURCE_DIR}/../main/frame/builder.cc
    ${CMAKE_SOURCE_DIR}/../main/frame/parser.cc
)

target_include_directories(test PRIVATE ${CMAKE_SOURCE_DIR}/../main/)

target_compile_options(test PRIVATE -Wall -Werror -Wextra -Wold-style-cast)

