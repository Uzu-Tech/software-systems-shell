#pragma once
///See reference for what these libraries provide
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>


///Constants for array sizes, defined for clarity and code readability
#define MAX_LINE 1024
#define MAX_ARGS 128
#define MAX_PROMPT_LEN PATH_MAX
#define PROMPT_TEXT "\033[1;35m$hell\033[0m: "
#define MAX_COMMANDS 10

///Enum for readable argument indices (use where required)
enum ArgIndex
{
    ARG_PROGNAME,
    ARG_1,
    ARG_2,
    ARG_3,
};

///With inline functions, the compiler replaces the function call 
///with the actual function code;
///inline improves speed and readability; meant for short functions (a few lines).
///the static here avoids linker errors from multiple definitions (needed with inline).
static inline void reap()
{
    wait(NULL);
}

///Shell I/O and related functions (add more as appropriate)
void read_command_line(char line[]);
void construct_shell_prompt(char shell_prompt[]);
void parse_command(char line[], char *args[], int *argsc);
///Program launching functions (add more as appropriate)
void launch_program(char *args[], int argsc);

void run_cd(char *args[], int argsc, char *lwd);
bool is_cd(char **args);