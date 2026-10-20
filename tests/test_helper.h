
#pragma once

#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "s3.h"




int run_child_and_get_exit_code(char *args[], int argsc);

void provide_input(const char *text);