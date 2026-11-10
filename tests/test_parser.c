#include <criterion/criterion.h>
#include <string.h>

#include "parser.h"
#include "shell.h"

#define NODE_CAPACITY 32
#define ARENA_CAPACITY 8192

typedef struct {
    char line[256];
    uint8_t arena_memory[ARENA_CAPACITY];
    Arena arena;
    ParserContext ctx;
} ParserFixture;

static void fixture_init(ParserFixture *fixture, const char *line)
{
    memset(fixture, 0, sizeof(*fixture));
    strncpy(fixture->line, line, sizeof(fixture->line) - 1);
    fixture->arena = (Arena){
        .base = fixture->arena_memory,
        .capacity = ARENA_CAPACITY
    };
    fixture->ctx = (ParserContext){
        .cursor = fixture->line,
        .arena = &fixture->arena
    };
}

Test(parser, parses_command_arguments)
{
    ParserFixture fixture;
    fixture_init(&fixture, "printf hello world");

    ASTNode *root = parse_line(&fixture.ctx);

    cr_assert_eq(root->id, 0);
    cr_assert_eq(root->type, AST_COMMAND);
    cr_assert_str_eq(root->command.argv[0], "printf");
    cr_assert_str_eq(root->command.argv[1], "hello");
    cr_assert_str_eq(root->command.argv[2], "world");
    cr_assert_null(root->command.argv[3]);
}

Test(parser, parses_pipeline_and_redirects)
{
    ParserFixture fixture;
    fixture_init(&fixture, "cat < input.txt | grep result >> output.txt");

    ASTNode *root = parse_line(&fixture.ctx);

    cr_assert_eq(root->type, AST_PIPELINE);
    cr_assert_eq(root->pipeline.count, 2);

    ASTNode *first = root->pipeline.children[0];
    cr_assert_eq(first->type, AST_REDIRECT);
    cr_assert_eq(first->redirect.redir_type, TOKEN_REDIR_IN);
    cr_assert_str_eq(first->redirect.file, "input.txt");

    ASTNode *second = root->pipeline.children[1];
    cr_assert_eq(second->type, AST_REDIRECT);
    cr_assert_eq(second->redirect.redir_type, TOKEN_REDIR_APPEND);
    cr_assert_str_eq(second->redirect.file, "output.txt");
}

Test(parser, parses_binary_operations)
{
    ParserFixture fixture;
    fixture_init(&fixture, "true && echo ok || echo failed");

    ASTNode *root = parse_line(&fixture.ctx);

    cr_assert_eq(root->type, AST_BINARY_OP);
    cr_assert_eq(root->bin_op.op_type, TOKEN_OR);

    ASTNode *left = root->bin_op.left;
    cr_assert_eq(left->type, AST_BINARY_OP);
    cr_assert_eq(left->bin_op.op_type, TOKEN_AND);
}

Test(parser, parses_batch)
{
    ParserFixture fixture;
    fixture_init(&fixture, "prepare; execute; cleanup");

    ASTNode *root = parse_line(&fixture.ctx);

    cr_assert_eq(root->type, AST_BATCH);
    cr_assert_eq(root->batch.count, 3);
    cr_assert_eq(root->batch.children[0]->id, 0);
    cr_assert_eq(root->batch.children[1]->id, 2);
    cr_assert_eq(root->batch.children[2]->id, 3);
}

Test(parser, parses_subshell)
{
    ParserFixture fixture;
    fixture_init(&fixture, "(echo nested)");

    ASTNode *root = parse_line(&fixture.ctx);

    cr_assert_eq(root->type, AST_SUBSHELL);
    ASTNode *child = root->subshell.child;
    cr_assert_eq(child->type, AST_COMMAND);
    cr_assert_str_eq(child->command.argv[0], "echo");
    cr_assert_eq(*fixture.ctx.cursor, '\0');
}

Test(parser, parses_nested_subshell)
{
    ParserFixture fixture;
    fixture_init(&fixture, "((echo nested))");

    ASTNode *root = parse_line(&fixture.ctx);

    cr_assert_eq(root->type, AST_SUBSHELL);

    ASTNode *inner = root->subshell.child;
    cr_assert_eq(inner->type, AST_SUBSHELL);

    ASTNode *command = inner->subshell.child;
    cr_assert_eq(command->type, AST_COMMAND);
    cr_assert_str_eq(command->command.argv[0], "echo");
    cr_assert_str_eq(command->command.argv[1], "nested");
    cr_assert_null(command->command.argv[2]);
    cr_assert_eq(*fixture.ctx.cursor, '\0');
}

Test(parser, parses_complex_command)
{
    ParserFixture fixture;
    fixture_init(
        &fixture,
        "(cat < input.txt | grep result > output.txt) && "
        "(echo success || echo failure); cleanup"
    );

    ASTNode *root = parse_line(&fixture.ctx);

    cr_assert_eq(root->type, AST_BATCH);
    cr_assert_eq(root->batch.count, 2);

    ASTNode *binary = root->batch.children[0];
    cr_assert_eq(binary->type, AST_BINARY_OP);
    cr_assert_eq(binary->bin_op.op_type, TOKEN_AND);

    ASTNode *left_subshell = binary->bin_op.left;
    cr_assert_eq(left_subshell->type, AST_SUBSHELL);
    ASTNode *left_pipeline = left_subshell->subshell.child;
    cr_assert_eq(left_pipeline->type, AST_PIPELINE);
    cr_assert_eq(left_pipeline->pipeline.count, 2);

    ASTNode *input_command = left_pipeline->pipeline.children[0];
    cr_assert_eq(input_command->type, AST_REDIRECT);
    cr_assert_eq(
        input_command->redirect.redir_type,
        TOKEN_REDIR_IN
    );
    cr_assert_str_eq(
        input_command->redirect.file,
        "input.txt"
    );

    ASTNode *output_command = left_pipeline->pipeline.children[1];
    cr_assert_eq(output_command->type, AST_REDIRECT);
    cr_assert_eq(
        output_command->redirect.redir_type,
        TOKEN_REDIR_OUT
    );
    cr_assert_str_eq(
        output_command->redirect.file,
        "output.txt"
    );

    ASTNode *right_subshell = binary->bin_op.right;
    cr_assert_eq(right_subshell->type, AST_SUBSHELL);
    ASTNode *right_binary = right_subshell->subshell.child;
    cr_assert_eq(right_binary->type, AST_BINARY_OP);
    cr_assert_eq(right_binary->bin_op.op_type, TOKEN_OR);

    ASTNode *cleanup = root->batch.children[1];
    cr_assert_eq(cleanup->type, AST_COMMAND);
    cr_assert_str_eq(cleanup->command.argv[0], "cleanup");
    cr_assert_eq(*fixture.ctx.cursor, '\0');
}

Test(parser, rejects_unclosed_subshell)
{
    ParserFixture fixture;
    fixture_init(&fixture, "(echo nested");

    ASTNode *root = parse_line(&fixture.ctx);

    cr_assert_eq(root, NODE_ERR);
    cr_assert_str_eq(fixture.ctx.err_msg, "Left parenthesis is not closed");
}

Test(parser, reports_arena_exhaustion)
{
    uint8_t arena_memory[sizeof(ASTNode) * 2];
    char line[] = "echo";
    Arena arena = {
        .base = arena_memory,
        .capacity = sizeof(arena_memory)
    };
    ParserContext ctx = {
        .cursor = line,
        .arena = &arena
    };

    cr_assert_eq(parse_line(&ctx), NODE_ERR);
    cr_assert_str_eq(ctx.err_msg, OOM_ERR_MSG);
}
