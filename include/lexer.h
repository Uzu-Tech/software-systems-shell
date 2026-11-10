#pragma once

typedef enum {
    TOKEN_REDIR_IN,
    TOKEN_REDIR_OUT,
    TOKEN_REDIR_APPEND,
    TOKEN_SEMI,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_PIPE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_WORD,
    TOKEN_EOL
} TokenType;

typedef struct {
    TokenType type;
    char* value;
    int length;
} Token;

Token lexer_consume(char **line);
static inline Token lexer_peek(char *line) { return lexer_consume(&line); };
bool is_redirect(TokenType type);