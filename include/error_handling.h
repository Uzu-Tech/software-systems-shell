#pragma once

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

void handle_wait_error(pid_t child_pid);
void handle_child_error(int status);