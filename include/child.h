#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "error.h"

typedef struct {
    char **args;
    int in_fd;
    int out_fd;
} ChildParams;

bool is_child(int rc);
Result reap_child(pid_t pid);
Result exec_program(const ChildParams* params);
