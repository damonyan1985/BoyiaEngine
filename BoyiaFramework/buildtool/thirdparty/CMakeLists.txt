cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wno-unused-parameter")

add_subdirectory(cjson)
add_subdirectory(tinyxml)
add_subdirectory(zip)