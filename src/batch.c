#pragma once
#include "s3.h"
#include "utils.h"

int handle_batch(char *line, char *lwd) {
    if (strchr(line, ';') == NULL) {
        return 0;
    }
    char *cmds[MAX_COMMANDS + 1];
    int count = split_string(line, cmds, MAX_COMMANDS + 1, ';');
    for (int i = 0; i < count; i++) {
        execute_line(cmds[i], lwd);
    }
    return 1;
}
