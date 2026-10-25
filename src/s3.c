#include "s3.h"
#include <stdio.h>
#include "error_handling.h"

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, "\033[1;35m$hell\033[0m: ");
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    ///See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL)
    {
        perror("fgets failed");
        exit(1);
    }
    ///Remove newline (enter)im 
    line[strcspn(line, "\n")] = '\0';
}

int split_string(char *line, char *args[], int capacity, char delimiter, bool keep_empty) {
    int count = 0;
    
    if (capacity < 1) {
        return -1;
    }
    args[0] = NULL;

    while (1) {
        char *separator = strchr(line, delimiter);

        if (separator != NULL) {
            *separator = '\0';
        }

        if (keep_empty || *line != '\0') {
            if (count == capacity - 1) {
                return -1;
            }

            args[count++] = line;
            args[count] = NULL;
        }

        if (separator == NULL) {
            return count;
        }

        line = separator + 1;
    }
}

void parse_command(char line[], char *args[], int *argsc)
{
    *argsc = split_string(line, args, MAX_ARGS, ' ', false);
}

///Launch related functions
void child(char *args[]) {
    if (execvp(args[0], args) < 0) {
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }
}

void wait_for_child(pid_t pid) {
    int status;

    if (waitpid(pid, &status, 0) == -1) {
        handle_wait_error(pid);
    }
}

void launch_program(char *args[], int argsc) {
    if (argsc < 0) {
        fprintf(stderr, "Too many arguments\n");
        return;
    }

    if (argsc == 0) {
        return;
    }

    pid_t rc = fork();

    if (rc < 0) {
        perror("fork has failed");
        return;
    }
    else if (rc == 0) {
        child(args);
    }
    else {
        wait_for_child(rc);
    }
}
