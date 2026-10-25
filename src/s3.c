#include "s3.h"
#include "child.h"
#include "utils.h"
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

// Prints a shell prompt and reads input from the user
void read_command_line(char line[])
{
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
