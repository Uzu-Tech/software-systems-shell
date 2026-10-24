#include "error_handling.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

void handle_negative_error(int result, const char* msg) {
    if (result < 0) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

void handle_null_error(const void* result, const char* msg) {
    if (result == NULL) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

void handle_wait_error(pid_t child_pid) {
    switch (errno) {
        case ECHILD:
            fprintf(stderr, "No child process with %d as process Id.\n", child_pid);
            break;
        case EINTR:
            fprintf(stderr, "Process %d interrupted by a signal.\n", child_pid);
            break;
        case EINVAL:
            fprintf(stderr, "Process %d has an illegal value passed into system call.\n", child_pid);
            break;
        default:
            perror("Wait failed with unknown error");
            break;
    }
}

void handle_child_error(int status){
    if(WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if (exit_code != 0) {
            fprintf(stderr, "Process failed with exit code %d", exit_code);
        }
        else if (WIFSIGNALED(status)){
            fprintf(stderr, "Process was killed by a signal %d\n", exit_code);
        }
    }
}