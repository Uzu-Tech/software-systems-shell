#include "arena.h"
#include "ast.h"
#include <stdlib.h>
#include <string.h>

static uint8_t* arena_alloc_align(Arena* arena, int bytes, int align) {
    int mask = align - 1;
    int aligned_start = (arena->byte_count + mask) & ~mask;
    if (aligned_start + bytes > arena->capacity) return NULL;
    arena->byte_count = aligned_start + bytes;
    return arena->base + aligned_start;
}

void arena_reset(Arena* arena) { arena->byte_count = 0; arena->node_count = 0; }

char* alloc_string(Arena* arena, int len) {
    return (char*)arena_alloc_align(arena, len * sizeof(char*), _Alignof(char*));
}

char* push_string(Arena* arena, char* string, int size) {
    char* dest = alloc_string(arena, size + 1);

    if (dest != NULL) {
        memcpy(dest, string, size);
        dest[size] = '\0';
    }

    return dest;
}

void* alloc_pointer(Arena* arena, int num_pointers, int type_size) {
    return (void*)arena_alloc_align(arena, num_pointers * sizeof(void*), _Alignof(void*));
}

ASTNode* alloc_node(Arena *arena, NodeType type) {
    ASTNode* new_node = (ASTNode*)arena_alloc_align(arena, sizeof(ASTNode), _Alignof(ASTNode));
    if (new_node == NULL) {
        return NODE_ERR;
    }

    new_node->id = arena->node_count++;
    new_node->type = type;
    return new_node;
}