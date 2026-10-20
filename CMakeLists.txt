cmake_minimum_required(VERSION 3.20)
project(Shell LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)