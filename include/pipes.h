#include "s3.h"
#include <stdio.h>
#include <string.h>

typedef void (*redirect_fn)(int[2]);

int split_on_pipe(char *line, char **left_str, char **right_str);
int prepare_pipeline_args(char *left_str, char *right_str, char *args1[], char *args2[]);

int create_pipe(int pipefd[2]);
void close_pipe(int pipefd[2]);
void redirect_pipe_writer(int pipefd[2]);
void redirect_pipe_reader(int pipefd[2]);
pid_t spawn_pipe_child(int pipefd[2], char *args[], redirect_fn redirect);
void wait_for_pipeline(pid_t writer_pid, pid_t reader_pid);

void execute_pipeline(char *args1[], char *args2[]);


#ifndef PIPES_H
#define PIPES_H
int handle_pipeline(char *line);

#endif