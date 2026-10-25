#include "child.h"
#include "redirect.h"
#include "error_handling.h"
#include <unistd.h>

bool is_child(int rc) {
    return rc == 0;
}

void execute_program(char* args[]) {
    exit_on_error(execvp(args[0], args), "execvp failed");
}

///Launch related functions
void child(const ChildParams* params)
{
    if (params->in_fd != -1) {
        redirect(STDIN_FILENO, params->in_fd);
    }
    if (params->out_fd != -1) {
        redirect(STDOUT_FILENO, params->out_fd);
    }

    execute_program(params->args);
}

void wait_for_child(pid_t pid) {
    int status;

    if (waitpid(pid, &status, 0) == -1) {
        handle_wait_error(pid);
    }
}