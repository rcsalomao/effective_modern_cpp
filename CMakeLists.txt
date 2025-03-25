cmake_minimum_required(VERSION 3.11...3.31)

project(
    effective_modern_cpp
    DESCRIPTION "This is a project description."
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
    effective_modern_cpp
    src/effective_modern_cpp.cpp
    src/item_1.cpp
    src/item_2.cpp
    src/item_3.cpp
    src/item_4.cpp
    src/item_5.cpp
    src/item_6.cpp
    src/item_7.cpp
    src/item_8.cpp
    src/item_9.cpp
    src/item_10.cpp
    src/item_11.cpp
    src/item_12.cpp
    src/item_13.cpp
    src/item_14.cpp
    src/item_15.cpp
    src/item_16.cpp
    src/item_17.cpp
    src/item_18.cpp
)

target_include_directories(
    effective_modern_cpp
    PRIVATE
    ~/.local/include
)

target_compile_options(
    effective_modern_cpp
    PRIVATE
    -fdiagnostics-color=always
    -Wall
    -Wextra
    -std=c++23
    -Og
    -march=native
)

target_link_directories(
    effective_modern_cpp
    PRIVATE
    ~/.local/lib
)

target_link_libraries(
    effective_modern_cpp
    PRIVATE
    -pthread
)

target_link_options(
    effective_modern_cpp
    PRIVATE
)
