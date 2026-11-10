
#pragma once

#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "child.h"

int run_program_and_get_exit_code(char *args[]);

void provide_input(const char *text);