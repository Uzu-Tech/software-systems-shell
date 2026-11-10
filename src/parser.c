#include "ast.h"
#include "lexer.h"
#include "shell.h"
#include "arena.h"
#include "parser.h"
#include "error.h"

#define PARSE_REQUIRE(ctx, cond, msg) \
    do { if (!(cond)) return parse_error((ctx), (msg)); } while (0)

#define PARSE_RETURN_IF_ERR(node) \
    do { if ((node) == NODE_ERR) return NODE_ERR; } while (0)

#define PARSE_RETURN_IF_NULL(node) \
    do { if ((node) == NODE_NULL) return NODE_NULL; } while (0)

int pipeline_results_slot = 0;

ASTNode* parse_error(ParserContext* ctx, const char* msg) {
    ctx->err_msg = msg;
    return NODE_ERR;
}

ASTNode* parse_subshell(ParserContext* ctx) {
    lexer_consume(&ctx->cursor); // Consume left bracket
    ASTNode* node = alloc_node(ctx->arena, AST_SUBSHELL);
    PARSE_REQUIRE(ctx, node != NODE_ERR, OOM_ERR_MSG);

    Subshell* subshell = &node->subshell;
    subshell->child = parse_batch(ctx, TOKEN_RPAREN);
    if (subshell->child == NODE_NULL) return NODE_NULL;
    if (subshell->child == NODE_ERR) return NODE_ERR;

    return node;
}

ASTNode* wrap_redirect(ParserContext* ctx, ASTNode* child_idx) {
    Token redir_token = lexer_consume(&ctx->cursor);

    ASTNode* node = alloc_node(ctx->arena, AST_REDIRECT);
    PARSE_REQUIRE(ctx, node != NODE_ERR, OOM_ERR_MSG);

    Redirect* redirect = &node->redirect;
    redirect->child = child_idx;
    redirect->redir_type = redir_token.type;

    Token file_token = lexer_consume(&ctx->cursor);
    PARSE_REQUIRE(ctx, file_token.type == TOKEN_WORD, "Expected filename after redirection operator");

    redirect->file = push_string(ctx->arena, file_token.value, file_token.length);
    PARSE_REQUIRE(ctx, redirect->file != NULL, OOM_ERR_MSG);
    return node;
}

ASTNode* parse_redirects_after_subshell(ParserContext* ctx, ASTNode* subshell) {
    PARSE_RETURN_IF_ERR(subshell);
    ASTNode* node = subshell;
    Token token = lexer_peek(ctx->cursor);

    while (is_redirect(token.type)) {
        node = wrap_redirect(ctx, node);
        PARSE_RETURN_IF_ERR(node);
        token = lexer_peek(ctx->cursor);
    }

    return node;
}


ASTNode* parse_single_command(ParserContext* ctx) {
    Token token = lexer_peek(ctx->cursor);

    if (token.type == TOKEN_EOL || token.type == TOKEN_RPAREN) return NODE_NULL;
    if (token.type == TOKEN_LPAREN) {
        return parse_redirects_after_subshell(ctx, parse_subshell(ctx));
    }
    PARSE_REQUIRE(
        ctx, token.type == TOKEN_WORD || is_redirect(token.type), "Invalid command"
    );

    ASTNode* node = alloc_node(ctx->arena, AST_COMMAND);
    PARSE_REQUIRE(ctx, node != NODE_ERR, OOM_ERR_MSG);
    
    Command* command = &node->command;

    command->argv = (char**)alloc_pointer(ctx->arena, MAX_ARGS + 1, sizeof(char*));
    PARSE_REQUIRE(ctx, command->argv != NULL, OOM_ERR_MSG);

    int arg_idx = 0;
    while (token.type == TOKEN_WORD || is_redirect(token.type)) {
        if (is_redirect(token.type)) {
            node = wrap_redirect(ctx, node);
            PARSE_RETURN_IF_ERR(node);
            token = lexer_peek(ctx->cursor);
            continue;
        }

        lexer_consume(&ctx->cursor);
        PARSE_REQUIRE(ctx, arg_idx != MAX_ARGS, "Too many arguments in one command");

        command->argv[arg_idx] = push_string(ctx->arena, token.value, token.length);
        PARSE_REQUIRE(ctx, command->argv[arg_idx] != NULL, OOM_ERR_MSG);
        arg_idx++;

        token = lexer_peek(ctx->cursor);
    }

    PARSE_REQUIRE(ctx, arg_idx > 0, "Redirect without a command");
    command->argv[arg_idx] = NULL;
    return node;
}

ASTNode* parse_pipeline(ParserContext* ctx) {
    ASTNode* first_cmd = parse_single_command(ctx);
    PARSE_RETURN_IF_ERR(first_cmd);
    PARSE_RETURN_IF_NULL(first_cmd);

    Token token = lexer_peek(ctx->cursor);
    if (token.type != TOKEN_PIPE) return first_cmd;

    ASTNode* node = alloc_node(ctx->arena, AST_PIPELINE);
    PARSE_REQUIRE(ctx, node != NODE_ERR, OOM_ERR_MSG);

    Pipeline* pipeline = &node->pipeline;

    pipeline->children = (ASTNode**)alloc_pointer(ctx->arena, MAX_COMMANDS, sizeof(ASTNode*));
    PARSE_REQUIRE(ctx, pipeline->children != NULL, OOM_ERR_MSG);
    pipeline->children[0] = first_cmd;
    pipeline->results_slot = pipeline_results_slot++;
    
    pipeline->count = 1;
    while (token.type == TOKEN_PIPE) {
        lexer_consume(&ctx->cursor);
        PARSE_REQUIRE(ctx, pipeline->count != MAX_COMMANDS, "Too many commands in one pipe");
        
        ASTNode* cmd = parse_single_command(ctx);
        PARSE_RETURN_IF_ERR(cmd);
        PARSE_REQUIRE(ctx, cmd != NODE_NULL, "Pipeline section must contain a command inside");

        pipeline->children[pipeline->count++] = cmd;
        token = lexer_peek(ctx->cursor);
    }

    return node;
}

ASTNode* parse_binary_op(ParserContext* ctx) {
    ASTNode* left = parse_pipeline(ctx);
    PARSE_RETURN_IF_ERR(left);
    PARSE_RETURN_IF_NULL(left);

    Token token = lexer_peek(ctx->cursor);

    while (token.type == TOKEN_OR || token.type == TOKEN_AND) {
        lexer_consume(&ctx->cursor);

        ASTNode* node = alloc_node(ctx->arena, AST_BINARY_OP);
        PARSE_REQUIRE(ctx, node != NODE_ERR, OOM_ERR_MSG);

        BinaryOp* bin_op = &node->bin_op;
        bin_op->op_type = token.type;
        bin_op->left = left;

        ASTNode* right = parse_pipeline(ctx);
        PARSE_RETURN_IF_ERR(right);
        PARSE_REQUIRE(ctx, right != NODE_NULL, "No command after binary operator");

        bin_op->right = right;
        left = node;
        token = lexer_peek(ctx->cursor);
    }

    return left;
}

ASTNode* parse_batch(ParserContext* ctx, TokenType return_type) {
    ASTNode* first_node = parse_binary_op(ctx);
    PARSE_RETURN_IF_ERR(first_node);
    PARSE_RETURN_IF_NULL(first_node);

    Token token = lexer_peek(ctx->cursor);
    if (token.type != TOKEN_SEMI) {
        if (return_type == TOKEN_RPAREN) {
            PARSE_REQUIRE(ctx, token.type == return_type, "Left parenthesis is not closed");
            lexer_consume(&ctx->cursor);
        } else {
            PARSE_REQUIRE(ctx, token.type == return_type, "Invalid tokens after command end");
        }
        return first_node;
    }

    ASTNode* node = alloc_node(ctx->arena, AST_BATCH);
    PARSE_REQUIRE(ctx, node != NODE_ERR, OOM_ERR_MSG);

    Batch* batch = &node->batch;

    batch->children = (ASTNode**)alloc_pointer(ctx->arena, MAX_COMMANDS, sizeof(ASTNode*));
    PARSE_REQUIRE(ctx, batch->children != NULL, OOM_ERR_MSG);
    batch->children[0] = first_node;
    
    batch->count = 1;
    while (token.type == TOKEN_SEMI) {
        lexer_consume(&ctx->cursor);

        if (lexer_peek(ctx->cursor).type == return_type) { // Trailing semi colon
            return node;
        }

        PARSE_REQUIRE(ctx, batch->count != MAX_COMMANDS, "Too many commands in one pipe");
        
        ASTNode* cmd = parse_single_command(ctx);
        PARSE_RETURN_IF_ERR(cmd);
        PARSE_REQUIRE(ctx, cmd != NODE_NULL, "Batch section must contain a command inside");

        batch->children[batch->count++] = cmd;
        token = lexer_peek(ctx->cursor);
    }

    if (return_type == TOKEN_RPAREN && token.type != return_type) {
        return parse_error(ctx, "Left parenthesis is not closed");
    }

    if (token.type != return_type) {
        return parse_error(ctx, "Invalid tokens after command end");
    }

    if (return_type == TOKEN_RPAREN) { // Consume left over RPAREN
        lexer_consume(&ctx->cursor);
    }

    return node;
}

ASTNode* parse_line(ParserContext *ctx) {
    return parse_batch(ctx, TOKEN_EOL);
}
