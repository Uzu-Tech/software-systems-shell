add_library(s3 s3.c redirect.c pipes.c error_handling.c)
target_include_directories(s3 PUBLIC ../include)

add_executable(app s3main.c)
target_link_libraries(app PRIVATE s3)