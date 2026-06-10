cmake_minimum_required(VERSION 3.21.2)

project(ECU_realtime)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add_subdirectory(libyaka)

add_executable(test
        test.cc
        frame/builder.cc
        frame/parser.cc
)
target_compile_options(test PRIVATE -Wall -Wextra -Werror -pedantic -Wold-style-cast)

# target_link_libraries(ecu PRIVATE yaka)

# target_include_directories(ecu PRIVATE ${CMAKE_SOURCE_DIR}/libyaka/include)
# target_include_directories(yaka PUBLIC ${CMAKE_SOURCE_DIR}/libyaka/include)
