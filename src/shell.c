#include "shell.h"
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "errno.h"

void construct_shell_prompt(char shell_prompt[]) {
    char cwd[MAX_PROMPT_LEN];

    if (getcwd(cwd, MAX_PROMPT_LEN) != NULL) {
        snprintf(shell_prompt, MAX_PROMPT_LEN, "\n\033[1;33m$hl@\033[0m\033[1;32m%s\033[0m $ ", cwd);
    } else {
        snprintf(shell_prompt, MAX_PROMPT_LEN, "\n\033[1;33m$hl@\033[0m $ ");
    }
}

bool read_command_line(char line[]) {
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    if (fgets(line, MAX_LINE, stdin) == NULL) {
        if (ferror(stdin)) {
            int saved = errno;
            fprintf(stderr, "fgets failed: %s", strerror(saved));
        }
        return false;
    }

    line[strcspn(line, "\n")] = '\0';
    return true;
}
