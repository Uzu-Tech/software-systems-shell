#include "pipes.h"
#include "redirect.h"
#include "child.h"
#include "utils.h"
#include "s3.h"
#include <stdio.h>
#include <string.h>

static int prepare_pipeline_args(char *cmds[], char *args[][MAX_ARGS], int count) {
    for (int i = 0; i < count; i++) {
        int argsc = 0;

        parse_command(cmds[i], args[i], &argsc);

        if (argsc < 0) {
            fprintf(stderr, "Too many arguments in pipeline command\n");
            return -1;
        }
    }

    return 0;
}

static int create_pipe(int pipefd[2]) {
    if (pipe(pipefd) < 0) {
        perror("pipe failed");
        return -1;
    }
    return 0;
}

static void close_pipe(int pipefd[2]) {
    close(pipefd[0]);
    close(pipefd[1]);
}

static void close_pipeline_pipes(int pipes[][2], int count) {
    for (int i = 0; i < count; i++) {
        close_pipe(pipes[i]);
    }
}

static void fill_pipe_fd(int pipes[][2], int pipe_count, int cmd_idx, int *in_fd, int *out_fd) {
    if (cmd_idx > 0) {
        *in_fd = pipes[cmd_idx - 1][0];
    }

    if (cmd_idx < pipe_count) {
        *out_fd = pipes[cmd_idx][1];
    }
}

static pid_t spawn_pipe_child(int pipes[][2], int pipe_count, int cmd_idx, char *args[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return -1;
    }

    if (is_child(pid)) {
        ChildParams params = {
            .args = args,
            .in_fd = -1,
            .out_fd = -1
        };

        if (cmd_idx == 0) {
            char* infile = NULL;
            extract_redirect(args, &infile, NULL);
            params.in_fd = (infile != NULL) ? open_infile(infile) : -1;
        }

        if (cmd_idx == pipe_count) {
            char* outfile = NULL;
            extract_redirect(args, NULL, &outfile);
            params.out_fd = (outfile != NULL) ? open_outfile(outfile) : -1;
        }

        fill_pipe_fd(pipes, pipe_count, cmd_idx, &params.in_fd, &params.out_fd);
        if (params.in_fd != -1) {
            redirect(STDIN_FILENO, params.in_fd);
        }
        if (params.out_fd != -1) {
            redirect(STDOUT_FILENO, params.out_fd);
        }
        close_pipeline_pipes(pipes, pipe_count);
        execute_program(params.args);
    }

    return pid;
}

void wait_for_pipeline(pid_t pids[], int count) {
    for (int i = 0; i < count; i++) {
        wait_for_child(pids[i]);
    }
}

static int create_pipeline_pipes(int pipes[][2], int pipe_count) {
    for (int i = 0; i < pipe_count; i++) {
        if (create_pipe(pipes[i]) < 0) {
            close_pipeline_pipes(pipes, i);
            return -1;
        }
    }
    return 0;
}

static int spawn_pipeline_children(int pipes[][2], char *args[][MAX_ARGS], int command_count, pid_t pids[]) {
    int pipe_count = command_count - 1;
    int num_children = 0;
    for (int i = 0; i < command_count; i++) {
        pid_t pid = spawn_pipe_child(pipes, pipe_count, i, args[i]);
        if (pid < 0) {
            break;
        }
        pids[num_children] = pid;
        num_children++;
    }
    return num_children;
}

static int execute_pipeline(char *args[][MAX_ARGS], int count) {
    int pipes[MAX_COMMANDS - 1][2];
    pid_t pids[MAX_COMMANDS];

    // Ignore NULL terminator at the end
    int pipe_count = count - 1;

    if (create_pipeline_pipes(pipes, pipe_count) < 0) {
        return -1;
    }
    int num_children = spawn_pipeline_children(pipes, args, count, pids);

    close_pipeline_pipes(pipes, pipe_count);
    wait_for_pipeline(pids, num_children);

    if (num_children != count) {
        return -1;
    }
    return 0;
}

int handle_pipeline(char *line) {
    if (strchr(line, '|') == NULL) {
        return 0;
    }

    char *cmds[MAX_COMMANDS + 1];
    char *args[MAX_COMMANDS][MAX_ARGS] = {0};

    int cmd_count = split_string(line, cmds, MAX_COMMANDS + 1, '|');

    if (cmd_count < 0) {
        fprintf(stderr, "Too many pipeline commands\n");
        return -1;
    }

    if (prepare_pipeline_args(cmds, args, cmd_count) < 0) {
        return -1;
    }

    if (execute_pipeline(args, cmd_count) < 0) {
        return -1;
    }

    return 1;
}
