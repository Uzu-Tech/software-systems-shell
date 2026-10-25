#include "s3.h"
#include "pipes.h"
#include "child.h"
#include "utils.h"
#include <fcntl.h>
#include <stdlib.h>
#include "redirect.h"
#include <stdio.h>
#include <limits.h>
#include "error_handling.h"

void construct_shell_prompt(char shell_prompt[]) {
    char cwd[MAX_PROMPT_LEN];

    if (getcwd(cwd, MAX_PROMPT_LEN) != NULL) {
        snprintf(shell_prompt, MAX_PROMPT_LEN, "\033[1;33m$hl@\033[0m\033[1;32m%s\033[0m $ ", cwd);
    } else {
        snprintf(shell_prompt, MAX_PROMPT_LEN, "\033[1;33m$hl@\033[0m $ ");
    }
}

bool is_cd(char **args) {
    return (args != NULL && args[0] != NULL && strcmp(args[0], "cd") == 0);
}

static bool save_cd (char *current_dir) {
    if (getcwd(current_dir, PATH_MAX) == NULL) {
        perror("cd: getcwd failed");
        return false;
    }
    return true;
}

static char *set_cd_argument(char *args[], int argsc, const char *lwd) {
    if (argsc == 1) {
        return getenv("HOME");
    } 
    if (strcmp(args[1], "-") == 0) {
        return (lwd[0] != '\0') ? lwd : NULL;
    } 
    return args[1];
}

static void set_cd(const char *target_dir, const char *current_dir, char *lwd) {
    if (chdir(target_dir) == 0) {
        strncpy(lwd, current_dir, PATH_MAX - 1);
        lwd[PATH_MAX - 1] = '\0';
    }
}

void run_cd(char *args[], int argsc, char *lwd) {
    char current_dir[PATH_MAX];
    if (!save_cd(current_dir)) return;
    char *target_dir = set_cd_argument(args, argsc, lwd);
    if (target_dir == NULL) return;
    set_cd(target_dir, current_dir, lwd);  
}

// Prints a shell prompt and reads input from the user
void read_command_line(char line[]) {
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);
    // Reads stdin into line
    exit_on_null(fgets(line, MAX_LINE, stdin), "fgets failed");
    // Remove newline
    line[strcspn(line, "\n")] = '\0';
}

void parse_command(char line[], char *args[], int *argsc) {
    *argsc = split_string(line, args, MAX_ARGS, ' ');
    // Handle argument overflow
    if (*argsc < 0) {
        *argsc = MAX_ARGS - 1;
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
    exit_on_error(rc, "fork has failed");

    if (is_child(rc)) {
        ChildParams params = {
            .args = args,
            .in_fd = -1,
            .out_fd = -1
        };

        char *infile = NULL, *outfile = NULL;
        extract_redirect(params.args, &infile, &outfile);
        if (infile != NULL) {
            params.in_fd = open_infile(infile);
        }
        if (outfile != NULL) {
            params.out_fd = open_outfile(outfile);
        }

        child(&params);
        exit(EXIT_FAILURE); // Fallback in case code somehow passes  
    }

    else {
        wait_for_child(rc);
    }
}
