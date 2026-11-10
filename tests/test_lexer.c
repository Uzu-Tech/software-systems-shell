#include <criterion/criterion.h>
#include <string.h>

#include "lexer.h"

static void assert_token(Token token, TokenType type, const char *value, int length)
{
    cr_assert_eq(token.type, type);
    cr_assert_eq(token.length, length);

    if (value == NULL) {
        cr_assert_null(token.value);
    } else {
        cr_assert_not_null(token.value);
        cr_assert_eq(strncmp(token.value, value, (size_t)length), 0);
    }
}

Test(lexer, tokenizes_words_and_whitespace)
{
    char line[] = "  echo   hello";
    char *cursor = line;

    assert_token(lexer_consume(&cursor), TOKEN_WORD, "echo", 4);
    assert_token(lexer_consume(&cursor), TOKEN_WORD, "hello", 5);
    assert_token(lexer_consume(&cursor), TOKEN_EOL, NULL, 0);
}

Test(lexer, tokenizes_single_and_double_operators)
{
    char line[] = "; | || & && > >> < ( )";
    char *cursor = line;
    const TokenType expected[] = {
        TOKEN_SEMI,
        TOKEN_PIPE,
        TOKEN_OR,
        TOKEN_AND,
        TOKEN_AND,
        TOKEN_REDIR_OUT,
        TOKEN_REDIR_APPEND,
        TOKEN_REDIR_IN,
        TOKEN_LPAREN,
        TOKEN_RPAREN,
        TOKEN_EOL
    };

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
        Token token = lexer_consume(&cursor);
        cr_assert_eq(token.type, expected[i]);
        int expected_length = i == 10 ? 0 : (i == 2 || i == 4 || i == 6 ? 2 : 1);
        cr_assert_eq(token.length, expected_length);
    }
}

Test(lexer, tokenizes_complex_command)
{
    char line[] =
        "(printf input < source.txt | grep pattern > output.txt) && "
        "(cat output.txt >> archive.log || echo fallback); "
        "cleanup < /tmp/status";
    char *cursor = line;
    const struct {
        TokenType type;
        const char *value;
        int length;
    } expected[] = {
        {TOKEN_LPAREN, "(", 1},
        {TOKEN_WORD, "printf", 6},
        {TOKEN_WORD, "input", 5},
        {TOKEN_REDIR_IN, "<", 1},
        {TOKEN_WORD, "source.txt", 10},
        {TOKEN_PIPE, "|", 1},
        {TOKEN_WORD, "grep", 4},
        {TOKEN_WORD, "pattern", 7},
        {TOKEN_REDIR_OUT, ">", 1},
        {TOKEN_WORD, "output.txt", 10},
        {TOKEN_RPAREN, ")", 1},
        {TOKEN_AND, "&&", 2},
        {TOKEN_LPAREN, "(", 1},
        {TOKEN_WORD, "cat", 3},
        {TOKEN_WORD, "output.txt", 10},
        {TOKEN_REDIR_APPEND, ">>", 2},
        {TOKEN_WORD, "archive.log", 11},
        {TOKEN_OR, "||", 2},
        {TOKEN_WORD, "echo", 4},
        {TOKEN_WORD, "fallback", 8},
        {TOKEN_RPAREN, ")", 1},
        {TOKEN_SEMI, ";", 1},
        {TOKEN_WORD, "cleanup", 7},
        {TOKEN_REDIR_IN, "<", 1},
        {TOKEN_WORD, "/tmp/status", 11},
        {TOKEN_EOL, NULL, 0}
    };

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
        assert_token(
            lexer_consume(&cursor),
            expected[i].type,
            expected[i].value,
            expected[i].length
        );
    }
}

Test(lexer, preserves_cursor_when_peeking)
{
    char line[] = "command next";
    char *cursor = line;

    Token peeked = lexer_peek(cursor);
    assert_token(peeked, TOKEN_WORD, "command", 7);
    cr_assert_eq(cursor, line);

    assert_token(lexer_consume(&cursor), TOKEN_WORD, "command", 7);
    assert_token(lexer_consume(&cursor), TOKEN_WORD, "next", 4);
}

Test(lexer, identifies_redirect_tokens)
{
    cr_assert(is_redirect(TOKEN_REDIR_IN));
    cr_assert(is_redirect(TOKEN_REDIR_OUT));
    cr_assert(is_redirect(TOKEN_REDIR_APPEND));
    cr_assert_not(is_redirect(TOKEN_WORD));
    cr_assert_not(is_redirect(TOKEN_PIPE));
}
