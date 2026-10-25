#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>

typedef struct {
    char **args;
    int in_fd;
    int out_fd;
} ChildParams;

bool is_child(int rc);
void wait_for_child(pid_t pid);
void execute_program(char* args[]);
void child(const ChildParams* params);
