#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// Macros and functions in implement C++ RAII ideas in C
static inline void auto_free(void** p) {
    if (*p != NULL) { free(*p); }
}

static inline void auto_close(int* fd) {
    if (*fd >= 0) { close(*fd); }
}

int split_string(char *line, char *args[], int capacity, char delimiter);

// Called when pointer goes out of scope
#define AUTO_FREE __attribute__((cleanup(auto_free)))
#define AUTO_CLOSE __attribute__((cleanup(auto_close)))