cmake_minimum_required(VERSION 3.20)
project(Shell LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.c")
add_executable(s3 s3main.c ${SOURCES})

target_include_directories(s3 PRIVATE include)