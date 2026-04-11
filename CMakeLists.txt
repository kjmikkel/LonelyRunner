cmake_minimum_required(VERSION 3.20)
project(LonelyRunner CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)

find_library(NTL_LIB NAMES ntl REQUIRED)
find_library(GMP_LIB NAMES gmp REQUIRED)
find_path(NTL_INCLUDE_DIR NAMES NTL/ZZ.h REQUIRED)

option(SANITIZE "Build with ASan + UBSan" OFF)
if(SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

enable_testing()

add_subdirectory(src/core)
add_subdirectory(src/app)
add_subdirectory(src/test)
add_subdirectory(src/fuzz)
