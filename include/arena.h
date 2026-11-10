#pragma once
#include <fcntl.h>
#include <unistd.h>
#include "ast.h"

#define ARENA_BYTES (1024 * 1024) // 1MB arena should be plenty

typedef struct {
    int byte_count;
    int capacity;
    int node_count;
    uint8_t* base;
} Arena;

char* alloc_string(Arena* arena, int len);
char* push_string(Arena* arena, char* string, int size);
void* alloc_pointer(Arena* arena, int num_pointers, int type_size);
ASTNode* alloc_node(Arena *arena, NodeType type);
void arena_reset(Arena* arena);