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
    src/item_19.cpp
    src/item_20.cpp
    src/item_21.cpp
    src/item_22.cpp
    src/item_22_Widget.cpp
    src/item_23.cpp
    src/item_24.cpp
    src/item_25.cpp
    src/item_26.cpp
    src/item_27.cpp
    src/item_28.cpp
    src/item_29.cpp
    src/item_30.cpp
    src/item_31.cpp
    src/item_32.cpp
    src/item_33.cpp
    src/item_34.cpp
    src/item_35.cpp
    src/item_36.cpp
    src/item_37.cpp
    src/item_38.cpp
    src/item_39.cpp
    src/item_40.cpp
    src/item_41.cpp
    src/item_42.cpp
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
