#pragma once
#include <stdbool.h>
#include <limits.h>
#include "error.h"

extern char lwd[];

bool is_cd(char **args);
Result run_cd(char *args[], char *lwd);