#include "test_helper.h"

int run_child_and_get_exit_code(char *args[], int argsc)
{
    pid_t pid = fork();
    cr_assert_neq(pid, -1, "fork failed");

    if (pid == 0) {
        child(args);
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

void provide_input(const char *text)
{
    cr_redirect_stdin();
    cr_redirect_stdout();

    FILE *input = cr_get_redirected_stdin();
    cr_assert_not_null(input);

    cr_assert_neq(fputs(text, input), EOF);
    cr_assert_eq(fclose(input), 0);
}