cmake_minimum_required(VERSION 3.21.2)

project(ECU_tests)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

add_executable(test
    test.c
    ${CMAKE_SOURCE_DIR}/../main/frame/builder.c
    ${CMAKE_SOURCE_DIR}/../main/frame/parser.c
)

target_include_directories(test PRIVATE ${CMAKE_SOURCE_DIR}/../main/)

target_compile_options(test PRIVATE -Wall -Werror -Wextra -Wvla)

