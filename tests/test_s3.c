#include <criterion/criterion.h>
#include "src/s3.c"

Test(launcher, passes_arguments_correctly, .timeout = 2.0)
{
    cr_redirect_stdout();

    char *args[] = {"echo", "hello world", "second", NULL};

    launch_program(args, 4);

    cr_assert_stdout_eq_str("hello world|second");
}