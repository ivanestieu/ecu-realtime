cmake_minimum_required(VERSION 3.10)
project(ECU_CruiseControl C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Activer les avertissements
add_compile_options(-Wall -Wextra -pedantic)

# Sources
set(SOURCES
    main.c
    frame/parser.c
    frame/builder.c
)

# Headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Exécutable
add_executable(ecu_test ${SOURCES})

# Enable testing
enable_testing()
add_test(NAME parser_builder_tests COMMAND ecu_test)
