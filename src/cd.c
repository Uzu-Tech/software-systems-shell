#include "cd.h"
#include "s3.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>


static bool save_cd (char *current_dir) {
    if (getcwd(current_dir, PATH_MAX) == NULL) {
        perror("cd: getcwd failed");
        return false;
    }
    return true;
}

static char *set_cd_argument(char *args[], int argsc, char *lwd) {
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

bool is_cd(char **args) {
    return (args != NULL && args[0] != NULL && strcmp(args[0], "cd") == 0);
}