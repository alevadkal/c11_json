cmake_minimum_required(VERSION 3.21)

include(common.cmake)

add_library(c11_json SHARED
    src/c11_json.c
)
target_include_directories(c11_json 
    PUBLIC 
        include
)

