#pragma once

#include "ast.h"
#include "utils.h"
#include "arena.h"

typedef struct {
    union {
        struct {
            char* cursor;
            Arena* arena;
        };
        const char* err_msg;
    };
} ParserContext;

ASTNode* parse_line(ParserContext* ctx);
ASTNode* parse_batch(ParserContext* ctx, TokenType return_type);