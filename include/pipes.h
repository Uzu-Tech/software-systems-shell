#pragma once

#include "error.h"

typedef struct {
    int read;
    int write;
} Pipe;

Result create_pipeline_pipes(Pipe pipes[], int pipe_count);
void close_pipeline_pipes(Pipe pipes[], int count);