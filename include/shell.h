#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MAX_LINE 1024
#define MAX_ARGS 128
#define MAX_PROMPT_LEN PATH_MAX
#define PROMPT_TEXT "\033[1;35m$hell\033[0m: "
#define MAX_COMMANDS 24

bool read_command_line(char line[]);
void construct_shell_prompt(char shell_prompt[]);