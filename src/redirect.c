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
    handle_negative_error(fd, "Failed to open input file");
    redirect(STDIN_FILENO, fd);
}

void redirect_stdout(const char* outfile) {
    AUTO_CLOSE int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    handle_negative_error(fd, "Failed to open output file");
    redirect(STDOUT_FILENO, fd);
}

bool str_is_equal(const char* s1, const char* s2) {
    return strcmp(s1, s2) == 0;
}

void remove_two_args(char *args[], int index) {
    int j = index;
    while (args[j + 2] != NULL) {
        args[j] = args[j + 2];
        j++;
    }
    args[j] = NULL;
}

void extract_redirect(char* args[], char** infile, char** outfile) {
    int i = 0;
    
    while (args[i] != NULL) { 
        if (str_is_equal(args[i], "<")) {
            *infile = args[i + 1];
            handle_null_error(*infile, "No file specified after < operator");
            remove_two_args(args, i);
        }

        else if (str_is_equal(args[i], ">")) {
            *outfile = args[i + 1];
            handle_null_error(*outfile, "No file specified after > operator");
            remove_two_args(args, i);
        }
        
        else {
            i++;
        }
    }
}