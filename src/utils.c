#include "utils.h"
#include "pipes.h"
#include "s3.h"
#include <string.h>

static inline bool does_exist(char *separator) { return separator != NULL; }

int split_string(char *line, char *args[], int capacity, char delimiter) {
    int count = 0;

    if (capacity < 1) {
        return -1;
    }

    args[0] = NULL;

    while (line != NULL) {
        char *separator = strchr(line, delimiter);
        if (does_exist(separator)) {
            *separator = '\0';
        }

        if (*line != '\0') {
            if (count >= capacity - 1) {
                args[count] = NULL;
                return -1;
            }
            args[count++] = line;
            args[count] = NULL;
        }

        line = does_exist(separator) ? separator + 1 : NULL;
    }

    return count;
}

void execute_line(char *line, char *lwd) {
    if (!handle_pipeline(line)) {
        
        char *args[MAX_ARGS] = {0};
        int argsc = 0;
        parse_command(line, args, &argsc);
        if (args[0] == NULL) { return;}
        if (is_cd(args)) { run_cd(args, argsc, lwd);}
        else { launch_program(args, argsc);}
    }
}