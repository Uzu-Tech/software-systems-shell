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

///Launch related functions
void child(char *args[])
{
    if (execvp(args[0], args) < 0) {
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }
}

void launch_program(char *args[], int argsc)
{
    pid_t rc = fork();

    if (rc < 0){ 
        perror("fork has failed");
        return;
    }
    else if (rc == 0){  //child process
        child(args);
        exit(EXIT_FAILURE);    
    }
    else{ //parent process
        int status;

        if (waitpid(rc, &status, 0) == -1) {
            handle_wait_error(rc);
        }
    }
}

