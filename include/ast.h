#pragma once

#include "lexer.h"
#include "error.h"

typedef int32_t ASTNodeIdx;
typedef struct ASTNode ASTNode; 

extern ASTNode ast_node_empty;
#define NODE_ERR NULL
#define NODE_NULL (&ast_node_empty)

typedef enum {
    AST_BATCH,
    AST_BINARY_OP,
    AST_PIPELINE,
    AST_REDIRECT,
    AST_COMMAND,
    AST_SUBSHELL,
    AST_NULL
} NodeType;

typedef struct {
    ASTNode** children;
    int count;
} Batch;

typedef struct {
    ASTNode* left;
    ASTNode* right;
    TokenType op_type;
} BinaryOp;

typedef struct {
    ASTNode** children;
    int count;
    int results_slot; // shared mmap region for pipeline child results
} Pipeline;

typedef struct {
    char** argv;
} Command;

typedef struct {
    ASTNode* child;
    TokenType redir_type;
    char* file;
} Redirect;

typedef struct {
    ASTNode* child;
} Subshell;

struct ASTNode {
    NodeType type;
    union {
        Batch batch;
        BinaryOp bin_op;
        Pipeline pipeline;
        Redirect redirect;
        Command command;
        Subshell subshell;
    };
    int id;
};