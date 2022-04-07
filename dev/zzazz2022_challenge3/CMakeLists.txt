cmake_minimum_required(VERSION 3.21)
project(zzazz2022_challenge3)

set(CMAKE_CXX_STANDARD 14)

if (MSVC)
    add_compile_options(/O3)
else ()
    add_compile_options(-O3)
endif ()

add_executable(zzazz2022_challenge3 main.cpp)
