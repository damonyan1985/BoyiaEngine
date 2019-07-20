cmake_minimum_required(VERSION 3.4.1)

project(bsdiff)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wno-unused-parameter")

file(
    GLOB 
    bsdiff_src
    "source/*.c"
    "source/*.cpp"
    )

add_library(
    bsdiff
    SHARED
    ${bsdiff_src}
)

target_include_directories(
    bsdiff
    PRIVATE
    include
    )