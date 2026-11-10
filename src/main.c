#include "ast.h"
#include "shell.h"
#include "arena.h"
#include "parser.h"
#include "execution.h"
#include "error.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    uint8_t arena_base[ARENA_BYTES];

    Arena arena = {
        .capacity = ARENA_BYTES,
        .base = arena_base
    };

    Result slot_result = error_slots_init(ARENA_BYTES / sizeof(ASTNode));
    if (is_error(slot_result)) {
        report_error(slot_result);
        return EXIT_FAILURE;
    }

    char line[MAX_LINE];

    for (;;) {
        fflush(STDIN_FILENO);
        if (!read_command_line(line)) break;

        ParserContext ctx = { .arena = &arena, .cursor = line };
        ASTNode* root = parse_line(&ctx);

        // Handle null and node errs here
        if (root == NODE_NULL) continue;
        if (root == NODE_ERR) {
            report_syntax_err(ctx.err_msg);
            arena_reset(&arena);
            continue;
        }

        Result res = execute_node(root, STDIN_FILENO, STDOUT_FILENO, EXEC_FORK);
        // Report any child node errs
        report_all_errors(arena.node_count);
        // Report main error
        report_error(res);

        reset_error_slots(arena.node_count);
        arena_reset(&arena);
    }

    return EXIT_SUCCESS;
}