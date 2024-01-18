cmake_minimum_required(VERSION 3.16)
project(test VERSION 1.0.0)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "-std=c++11 ${SSE_FLAGS} -g -O3")

add_subdirectory(src)
install(DIRECTORY data DESTINATION ${PROJECT_SOURCE_DIR}/exe)
