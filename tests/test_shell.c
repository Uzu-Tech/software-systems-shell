#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stdio.h>
#include <unistd.h>
#include "shell.h"
#include "test_helper.h"

#define TEST_TIMEOUT_SECONDS 2.0


Test(launcher, preserves_arguments, .timeout = TEST_TIMEOUT_SECONDS)
{
    cr_redirect_stdout();

    char *args[] = {
        "printf", "[%s]\n", "hello world", "", "after", NULL
    };

    cr_assert_eq(run_program_and_get_exit_code(args), EXIT_SUCCESS);

    cr_assert_stdout_eq_str("[hello world]\n[]\n[after]\n");
}

// Test argument with gap passed correctly
Test(launcher, passes_arguments_correctly, .timeout = TEST_TIMEOUT_SECONDS)
{
    cr_redirect_stdout();

    char *args[] = {"echo", "hello world", NULL};
    cr_assert_eq(run_program_and_get_exit_code(args), EXIT_SUCCESS);

    cr_assert_stdout_eq_str("hello world\n");
}

// Test empty arguments passed correctly
Test(launcher, empty_argument, .timeout = TEST_TIMEOUT_SECONDS){

    cr_redirect_stdout();

    char *args[] = {"echo", "", NULL};
    cr_assert_eq(run_program_and_get_exit_code(args), EXIT_SUCCESS);

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

Test(child_process, runs_command, .timeout = TEST_TIMEOUT_SECONDS)
{
    cr_redirect_stdout();

    char *args[] = {"printf", "hello", NULL};

    int code = run_program_and_get_exit_code(args);

    cr_assert_eq(code, EXIT_SUCCESS);
    cr_assert_stdout_eq_str("hello");
}

Test(child_process, reports_missing_command, .timeout = TEST_TIMEOUT_SECONDS)
{
    cr_redirect_stderr();

    char *args[] = {"/__s3_nonexistent_test_command__", NULL};

    int code = run_program_and_get_exit_code(args);

    cr_assert_eq(code, EXIT_FAILURE);
    cr_assert_stderr_eq_str("");
}
