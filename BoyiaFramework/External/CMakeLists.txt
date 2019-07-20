cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wno-unused-parameter")

add_subdirectory(bsdiff)
add_subdirectory(zlib)
add_subdirectory(lpng)
add_subdirectory(jpeg)
add_subdirectory(ft2)
add_subdirectory(ssl)
add_subdirectory(curl)