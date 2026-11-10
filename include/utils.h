#pragma once
#include <fcntl.h>
#include <unistd.h>
#include "error.h"

int split_string(char *line, char *args[], int capacity, char delimiter);
void execute_line(char *line, char *lwd);

Result redirect(int prev_fd, int new_fd);
Result redirect_stdio(int in_fd, int out_fd);

// Called when pointer goes out of scope
#define AUTO_FREE __attribute__((cleanup(auto_free)))
#define AUTO_CLOSE __attribute__((cleanup(auto_close)))