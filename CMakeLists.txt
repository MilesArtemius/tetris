cmake_minimum_required(VERSION 3.12)
project(tetris C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(tetris tetris.c tetris.h)
target_link_libraries(tetris pthread)