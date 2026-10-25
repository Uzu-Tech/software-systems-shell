#pragma once

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

void handle_wait_error(pid_t child_pid);
void handle_child_error(int status);
void exit_on_error(int result, const char* msg);
void exit_on_null(const void* result, const char* msg);