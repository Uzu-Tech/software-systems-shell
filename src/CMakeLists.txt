add_library(s3 shell.c cd.c pipes.c error.c utils.c child.c lexer.c ast.c parser.c arena.c execution.c)
target_include_directories(s3 PUBLIC ../include)

add_executable(app main.c)
target_link_libraries(app PRIVATE s3)