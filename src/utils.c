#include "utils.h"
#include "error.h"

Result redirect(int prev_fd, int new_fd) {
    if (prev_fd == new_fd) {
        return result_ok();
    }

    RETURN_ON_SYS_ERR(dup2(new_fd, prev_fd), ERR_DUP2);
    return result_ok();
}

Result redirect_stdio(int in_fd, int out_fd) {
    RETURN_IF_ERR(redirect(STDIN_FILENO, in_fd));
    return redirect(STDOUT_FILENO, out_fd);
}