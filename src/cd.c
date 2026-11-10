#include "cd.h"
#include "shell.h"
#include "error.h"
#include <linux/limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

char lwd[PATH_MAX] = "";

static Result save_cd(char *current_dir) {
    RETURN_ON_NULL_ERR(getcwd(current_dir, PATH_MAX), ERR_CD);
    return result_ok();
}

static char *get_cd_argument(char *args[], char *lwd) {
    if (args[1] == NULL) {
        return getenv("HOME");
    } 
    if (strcmp(args[1], "-") == 0) {
        return (lwd[0] != '\0') ? lwd : NULL;
    } 
    return args[1];
}

static Result set_cd(const char *target_dir, const char *current_dir, char *lwd) {
    RETURN_ON_SYS_ERR(chdir(target_dir), ERR_CD);
    strncpy(lwd, current_dir, PATH_MAX - 1);
    lwd[PATH_MAX - 1] = '\0';
    return result_ok();
}

Result run_cd(char *args[], char *lwd) {
    char current_dir[PATH_MAX];
    RETURN_IF_ERR(save_cd(current_dir));
    char *target_dir = get_cd_argument(args, lwd);
    if (target_dir == NULL) return result_ok();
    return set_cd(target_dir, current_dir, lwd);  
}

bool is_cd(char **args) {
    return (args != NULL && args[0] != NULL && strcmp(args[0], "cd") == 0);
}