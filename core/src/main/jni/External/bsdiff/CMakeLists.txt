cmake_minimum_required(VERSION 3.4.1)


file(
    GLOB 
    bsdiff_src
    "source/*.cpp"
    )

add_library(
    bsdiff
    SHARED
    ${bsdiff_src}
)

target_include_directories(
    boyia
    PRIVATE
    include
    )   