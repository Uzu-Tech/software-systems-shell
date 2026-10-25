#pragma once
#include <stdbool.h>
#include <limits.h>



bool is_cd(char **args);
void run_cd(char *args[], int argsc, char *lwd);