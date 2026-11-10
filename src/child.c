#include "child.h"
#include "error.h"
#include "errno.h"
#include "utils.h"
#include <unistd.h>

bool is_child(int rc) {
    return rc == 0;
}

Result exec_program(const ChildParams* params) {
    RETURN_IF_ERR(redirect_stdio(params->in_fd, params->out_fd));
    int rc = execvp(params->args[0], params->args);
    return (Result){ .type = ERR_EXEC, .err_num = errno, .context = params->args[0] };
}

Result reap_child(pid_t pid) {
    int status;
    pid_t rc;

    do {
        rc = waitpid(pid, &status, 0);
    } while (rc < 0 && errno == EINTR);   // interrupted: retry, not an error

    RETURN_ON_SYS_ERR(rc, ERR_WAIT);
    return get_child_result(status);
}