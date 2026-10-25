#include "s3.h"
#include <fcntl.h>
#include <stdlib.h>
#include "redirect.h"
#include <stdio.h>
#include "error_handling.h"

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, PROMPT_TEXT);
}

///Prints a shell prompt and    reads input from the user
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
    ///Remove newline (enter)
    line[strcspn(line, "\n")] = '\0';

    // "echo\0hello\0world \0"
}

void parse_command(char line[], char *args[], int *argsc)
{

    char *token = strtok(line, " "); //first space replaced with null terminator
    *argsc = 0;
    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        args[*argsc] = token;
        (*argsc)++; 
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL; //args must be null terminated
}

static inline bool is_child(int rc) {
    return (rc == 0);
}

static void execute_program(char* args[]) {
    exit_on_error(execvp(args[0], args), "execvp failed");
}

// Launch related functions
void child(const ChildParams* params)
{
    if (params->infile != NULL) {
        redirect_stdin(params->infile);
    }
    if (params->outfile != NULL) {
        redirect_stdout(params->outfile);
    }

    execute_program(params->args);
}

void launch_program(char *args[], int argsc)
{
    pid_t rc = fork();
    exit_on_error(rc, "fork has failed");

    if (is_child(rc)) {
        ChildParams params = {
            .args = args,
            .infile = NULL,
            .outfile = NULL
        };

        extract_redirect(params.args, &params.infile, &params.outfile);

        child(&params);
        exit(EXIT_FAILURE); // Fallback in case code somehow passes  
    }

    else { // parent process
        int status;

        if (waitpid(rc, &status, 0) == -1) {
            handle_wait_error(rc);
        }

        if (WIFEXITED(status)) {
            handle_child_error(status);
        }
    }
}