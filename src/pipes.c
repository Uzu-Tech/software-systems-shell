#include "pipes.h"

int split_on_pipe(char *line, char **left_str, char **right_str) {
    char *pipe_ptr = strchr(line, '|');
    if (pipe_ptr == NULL) {
        return 0;
    }

    *pipe_ptr = '\0';
    *left_str = line;
    *right_str = pipe_ptr + 1;

    if (**left_str == '\0' || **right_str == '\0') {
        fprintf(stderr, "Syntax error near unexpected token '|'\n");
        return -1;
    }

    return 1;
}

int prepare_pipeline_args(char *left_str, char *right_str, char *args1[], char *args2[]) {
    int argsc1 = 0;
    int argsc2 = 0;

    parse_command(left_str, args1, &argsc1);
    parse_command(right_str, args2, &argsc2);

    if (argsc1 == 0 || argsc2 == 0) {
        fprintf(stderr, "Syntax error: missing command near '|'\n");
        return -1;
    }
    return 0; 
}

int create_pipe(int pipefd[2]) {
    if (pipe(pipefd) < 0) {
        perror("pipe failed");
        return -1;
    }
    return 0;
}

void close_pipe(int pipefd[2]) {
    close(pipefd[0]);
    close(pipefd[1]);
}

void redirect_pipe_writer(int pipefd[2]) {
    dup2(pipefd[1], STDOUT_FILENO);
    close_pipe(pipefd);
}

void redirect_pipe_reader(int pipefd[2]) {
    dup2(pipefd[0], STDIN_FILENO);
    close_pipe(pipefd);
}

pid_t spawn_pipe_child(int pipefd[2], char *args[], redirect_fn redirect) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return -1;
    }

    if (pid == 0) {
        redirect(pipefd);
        child(args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }

    return pid;
}

void wait_for_pipeline(pid_t writer_pid, pid_t reader_pid) {
    waitpid(writer_pid, NULL, 0);
    waitpid(reader_pid, NULL, 0);
}

void execute_pipeline(char *args1[], char *args2[]) {
    int pipefd[2];
    if (create_pipe(pipefd) < 0) return;

    pid_t writer_pid = spawn_pipe_child(pipefd, args1, redirect_pipe_writer);
    pid_t reader_pid = spawn_pipe_child(pipefd, args2, redirect_pipe_reader);

    close_pipe(pipefd);
    wait_for_pipeline(writer_pid, reader_pid);
}

int handle_pipeline(char *line) {
    char *left_str = NULL;
    char *right_str = NULL;

    int status = split_on_pipe(line, &left_str, &right_str);
    if (status <= 0) {
        return status;
    }

    char *args1[MAX_ARGS];
    char *args2[MAX_ARGS];
    if (prepare_pipeline_args(left_str, right_str, args1, args2) < 0) {
        return -1;
    }

    execute_pipeline(args1, args2);

    return 1;
}