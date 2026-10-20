#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "s3.h"

#define TEST_TIMEOUT_SECONDS 2.0
#define MAX_LINE 100

static int run_child_and_get_exit_code(char *args[], int argsc)
{
    pid_t pid = fork();
    cr_assert_neq(pid, -1, "fork failed");

    if (pid == 0) {
        child(args, argsc);
        _exit(99);  // Detect an unexpected return from child().
    }

    int status;
    pid_t result;

    do {
        result = waitpid(pid, &status, 0);
    } while (result == -1 && errno == EINTR);

    cr_assert_eq(result, pid);
    cr_assert(WIFEXITED(status), "Child terminated by a signal");

    return WEXITSTATUS(status);
}

static void provide_input(const char *text)
{
    cr_redirect_stdin();
    cr_redirect_stdout();

    FILE *input = cr_get_redirected_stdin();
    cr_assert_not_null(input);

    cr_assert_neq(fputs(text, input), EOF);
    cr_assert_eq(fclose(input), 0);
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
Test(command_input, reads_one_line_at_a_time,
     .timeout = TEST_TIMEOUT_SECONDS)
{
    provide_input("pwd\nls\n");

    char first[MAX_LINE];
    char second[MAX_LINE];

    read_command_line(first);
    read_command_line(second);

    cr_assert_str_eq(first, "pwd");
    cr_assert_str_eq(second, "ls");
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
