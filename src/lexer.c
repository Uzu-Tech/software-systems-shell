#include "lexer.h"
#include <ctype.h>
#include <string.h>

bool is_operator(char c) {
    const char* ops = ";|&><()";
    for (;;) {
        if (*ops == c) return true;
        if (*ops == '\0') return false;
        ops++;
    }
}

bool is_redirect(TokenType type) {
    return (
        type == TOKEN_REDIR_APPEND
        || type == TOKEN_REDIR_IN
        || type == TOKEN_REDIR_OUT
    );
}

TokenType get_double_operator(char *token) {
    if (token[0] == '|' && token[1] == '|') {
        return TOKEN_OR;
    }
    if (token[0] == '&' && token[1] == '&') {
        return TOKEN_AND;
    }
    if (token[0] == '>' && token[1] == '>') {
        return TOKEN_REDIR_APPEND;
    }
    return TOKEN_EOL;
}

static TokenType get_single_token_type(char c) {
    switch (c) {
        case ';': return TOKEN_SEMI;
        case '|': return TOKEN_PIPE;
        case '&': return TOKEN_AND;
        case '>': return TOKEN_REDIR_OUT;
        case '<': return TOKEN_REDIR_IN;
        case '(': return TOKEN_LPAREN;
        case ')': return TOKEN_RPAREN;
    }

    return TOKEN_WORD;
}

Token lexer_consume(char **line) {
    while (isspace(**line)) (*line)++;

    if (**line == '\0') return (Token){
        .type = TOKEN_EOL, .value = NULL, .length = 0
    };

    char* curr_str = *line;

    if (is_operator(*curr_str)) {
        TokenType double_op = get_double_operator(curr_str);
        if (double_op != TOKEN_EOL) {
            (*line) += 2;
            return (Token) {
                .type = double_op, .value = curr_str, .length = 2
            };
        }

        (*line)++;
        return (Token) {
            .type = get_single_token_type(*curr_str), .value = curr_str, .length = 1
        };
    }

    // Must be a command arg by this point
    int len = 0;
    while (**line != '\0' && !isspace(**line) && !is_operator(**line)) {
        (*line)++;
        len++;
    }

    return (Token){
        .type = TOKEN_WORD,
        .value = curr_str,
        .length = len
    };
}