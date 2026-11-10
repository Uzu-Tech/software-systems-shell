#include "pipes.h"
#include "error.h"
#include "shell.h"
#include <sys/mman.h>
#include <errno.h>

static Result create_pipe(Pipe* p) {
    int fd[2];
    RETURN_ON_SYS_ERR(pipe(fd), ERR_PIPE);
    p->read = fd[0];
    p->write = fd[1];
    return result_ok();
}

static void close_pipe(Pipe* p) {
    close(p->read);
    close(p->write);
}

void close_pipeline_pipes(Pipe pipes[], int count) {
    for (int i = 0; i < count; i++) {
        close_pipe(&pipes[i]);
    }
}

Result create_pipeline_pipes(Pipe pipes[], int pipe_count) {
    for (int i = 0; i < pipe_count; i++) {
        Result res = create_pipe(&pipes[i]);
        if (is_error(res)) {
            close_pipeline_pipes(pipes, i);
            return res;
        }
    }
    return result_ok();
}