#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "s3.h"
#include "test_helper.h"

#define TEST_TIMEOUT_SECONDS 2.0


Test(launcher, preserves_arguments, .timeout = TEST_TIMEOUT_SECONDS)
{
    cr_redirect_stdout();

    char *args[] = {
        "printf", "[%s]\n", "hello world", "", "after", NULL
    };

    launch_program(args, 5);

    cr_assert_stdout_eq_str("[hello world]\n[]\n[after]\n");
}

// Test argument with gap passed correctly
Test(launcher, passes_arguments_correctly, .timeout = TEST_TIMEOUT_SECONDS)
{
    cr_redirect_stdout();

    char *args[] = {"echo", "hello world", NULL};
    launch_program(args, 2);

    cr_assert_stdout_eq_str("hello world\n");
}

// Test empty arguments passed correctly
Test(launcher, empty_argument, .timeout = TEST_TIMEOUT_SECONDS){

    cr_redirect_stdout();

    char *args[] = {"echo", "", NULL};
    launch_program(args, 2);

    cr_assert_stdout_eq_str("\n");
}

// read command line accepts and empty line
Test(command_input, accepts_empty_line, .timeout = TEST_TIMEOUT_SECONDS)
{
    provide_input("\n");

    char line[MAX_LINE];
    read_command_line(line);

    cr_assert_str_eq(line, "");
}

// read command line reads one line at a time
Test(command_input, reads_one_line_at_a_time, .timeout = TEST_TIMEOUT_SECONDS)
{
    provide_input("pwd\nls\n");

    char first[MAX_LINE];
    char second[MAX_LINE];

    read_command_line(first);
    read_command_line(second);

    cr_assert_str_eq(first, "pwd");
    cr_assert_str_eq(second, "ls");
}

Test(command_input, preserves_last_character, .timeout = TEST_TIMEOUT_SECONDS)
{
    provide_input("pwd");

    char line[MAX_LINE];
    read_command_line(line);

    cr_assert_str_eq(line, "pwd");
}

Test(parser, handles_extra_spaces, .timeout = TEST_TIMEOUT_SECONDS)
{
    char line[] = "  ls   -l  ";
    char *args[MAX_ARGS] = {NULL};
    int argsc = -1;

    // Make sure the parser actually writes the terminator.
    args[2] = line;

    parse_command(line, args, &argsc);

    cr_assert_eq(argsc, 2);
    cr_assert_str_eq(args[0], "ls");
    cr_assert_str_eq(args[1], "-l");
    cr_assert_null(args[2]);
}

Test(parser, handles_empty_input, .timeout = TEST_TIMEOUT_SECONDS)
{
    char line[] = "";
    char *args[MAX_ARGS] = {line};
    int argsc = 42;

    parse_command(line, args, &argsc);

    cr_assert_eq(argsc, 0);
    cr_assert_null(args[0]);
}

Test(parser, limits_argument_count, .timeout = TEST_TIMEOUT_SECONDS)
{
    // Build MAX_ARGS tokens: one more than the parser accepts.
    char line[2 * MAX_ARGS + 1];

    for (int i = 0; i < MAX_ARGS; i++) {
        line[2 * i] = 'x';
        line[2 * i + 1] = ' ';
    }
    line[2 * MAX_ARGS] = '\0';

    char *args[MAX_ARGS] = {NULL};
    args[MAX_ARGS - 1] = line; // Detect a missing NULL terminator.
    int argsc = -1;

    parse_command(line, args, &argsc);

    cr_assert_eq(argsc, MAX_ARGS - 1);

    for (int i = 0; i < argsc; i++) {
        cr_assert_str_eq(args[i], "x");
    }

    cr_assert_null(args[argsc]);
}

Test(child_process, runs_command, .timeout = TEST_TIMEOUT_SECONDS)
{
    cr_redirect_stdout();

    char *args[] = {"printf", "hello", NULL};

    int code = run_child_and_get_exit_code(args, 2);

    cr_assert_eq(code, EXIT_SUCCESS);
    cr_assert_stdout_eq_str("hello");
}

Test(child_process, reports_missing_command, .timeout = TEST_TIMEOUT_SECONDS)
{
    cr_redirect_stderr();

    char *args[] = {"/__s3_nonexistent_test_command__", NULL};

    int code = run_child_and_get_exit_code(args, 1);

    cr_assert_eq(code, EXIT_FAILURE);
    cr_assert_stderr_neq_str("");
}
