#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include "s3.h"

Test(launcher, passes_arguments_correctly, .timeout = 2.0)
{
    cr_redirect_stdout();

    char *args[] = {"echo", "hello world", NULL};
    launch_program(args, 2);

    cr_assert_stdout_eq_str("hello world\n");
}