#include "pipes.h"

static int split_on_pipes(char *line, char *parts[]) {
    return split_string(line, parts, MAX_COMMANDS + 1, '|', true);
}

static int prepare_pipeline_args(char *parts[], char *args[][MAX_ARGS], int count) {
    for (int i = 0; i < count; i++) {
        int argsc = 0;

        parse_command(parts[i], args[i], &argsc);

        if (argsc < 0) {
            fprintf(stderr, "Too many arguments in pipeline command\n");
            return -1;
        }

        if (argsc == 0) {
            fprintf(stderr, "Syntax error: missing command near '|'\n");
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

static int redirect_pipe_child(int pipes[][2], int pipe_count, int command_index) {
    if (command_index > 0) {
        if (dup2(pipes[command_index - 1][0], STDIN_FILENO) < 0) {
            return -1;
        }
    }
    if (command_index < pipe_count) {
        if (dup2(pipes[command_index][1], STDOUT_FILENO) < 0) {
            return -1;
        }
    }
    return 0;
}
static pid_t spawn_pipe_child(int pipes[][2], int pipe_count, int command_index, char *args[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return -1;
    }

    if (pid == 0) {
        if (redirect_pipe_child(pipes, pipe_count, command_index) < 0) {
            perror("dup2 failed");
            _exit(EXIT_FAILURE);
        }
        close_pipeline_pipes(pipes, pipe_count);
        child(args);
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

static int spawn_pipeline_children(int pipes[][2], char *args[][MAX_ARGS], int count, pid_t pids[]) {
    int started = 0;
    for (int i = 0; i < count; i++) {
        pid_t pid = spawn_pipe_child(pipes, count - 1, i, args[i]);
        if (pid < 0) {
            break;
        }
        pids[started++] = pid;
    }
    return started;
}

static int execute_pipeline(char *args[][MAX_ARGS], int count) {
    int pipes[MAX_COMMANDS - 1][2];
    pid_t pids[MAX_COMMANDS];

    int pipe_count = count - 1;

    if (create_pipeline_pipes(pipes, pipe_count) < 0) {
        return -1;
    }
    int started = spawn_pipeline_children(pipes, args, count, pids);

    close_pipeline_pipes(pipes, pipe_count);
    wait_for_pipeline(pids, started);

    if (started != count) {
        return -1;
    }
    return 0;
}

int handle_pipeline(char *line) {
    if (strchr(line, '|') == NULL) {
        return 0;
    }

    char *parts[MAX_COMMANDS + 1];
    char *args[MAX_COMMANDS][MAX_ARGS] = {0};

    int count = split_on_pipes(line, parts);

    if (count < 0) {
        fprintf(stderr, "Too many pipeline commands\n");
        return -1;
    }
    if (prepare_pipeline_args(parts, args, count) < 0) {
        return -1;
    }
    if (execute_pipeline(args, count) < 0) {
        return -1;
    }
    return 1;
}
