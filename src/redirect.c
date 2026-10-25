#include "redirect.h"
#include "error_handling.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"

void redirect(int prev_fd, int new_fd) {
    if (dup2(new_fd, prev_fd) < 0) {
        perror("Dup2 syscall failed");
        exit(EXIT_FAILURE);
    }
}

void redirect_stdin(const char* infile) {
    AUTO_CLOSE int fd = open(infile, O_RDONLY);
    exit_on_error(fd, "Failed to open input file");
    redirect(STDIN_FILENO, fd);
}

void redirect_stdout(const char* outfile) {
    AUTO_CLOSE int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    exit_on_error(fd, "Failed to open output file");
    redirect(STDOUT_FILENO, fd);
}

static bool str_is_equal(const char* s1, const char* s2) {
    return strcmp(s1, s2) == 0;
}

static void remove_two_args(char *args[], int index) {
    int j = index;
    while (args[j + 2] != NULL) {
        args[j] = args[j + 2];
        j++;
    }
    args[j] = NULL;
}

static void extract_redirect_file(char** file, char* args[], int i) {
    *file = args[i + 1];
    exit_on_null(file, "No file specified after redirection operator");
    remove_two_args(args, i);
}

void extract_redirect(char* args[], char** infile, char** outfile) {
    int i = 0;
    
    while (args[i] != NULL) { 
        if (str_is_equal(args[i], "<")) {
            extract_redirect_file(infile, args, i);
        }
        else if (str_is_equal(args[i], ">")) {
            extract_redirect_file(outfile, args, i);
        }
        else {
            i++;
        }
    }
}