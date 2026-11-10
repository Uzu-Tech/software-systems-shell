#include "error.h"
#include "errno.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static Result* error_slots = NULL;
static int slot_count = 0;

// { text, use_errno }
const ErrInfo err_info[ERR_COUNT] = {
    [ERR_NONE] = {NULL, false},
    [ERR_FORK] = {"fork failed", true},
    [ERR_PIPE] = { "pipe failed", true},
    [ERR_DUP2] = {"cannot redirect", true},
    [ERR_OPEN] = { NULL, true},
    [ERR_CLOSE] = {"close failed", true},
    [ERR_WAIT] = {"wait failed", true},
    [ERR_CD] = {"cd failed", true},
    [ERR_MMAP] = {"cannot allocate shared memory", true},
    [ERR_EXEC] = {NULL, true},
    [ERR_SIGNALED] = {NULL, true}
};

Result result_sys(ErrCode code) {
    uint8_t saved = errno; 
    return (Result){
        .type = (uint8_t)code,
        .err_num = saved,
    };
}

Result get_child_result(int status){
    if (WIFEXITED(status)) { // Child exited normally
        return result_exit(WEXITSTATUS(status));
    }

    if (WIFSIGNALED(status)) { // Child was killed by a signal
        return (Result){.type = ERR_SIGNALED, .signal_code = WTERMSIG(status)};
    }

    return result_exit(EXIT_FAILURE);
}

Result error_slots_init(int count) {
    void* p = mmap(NULL, count * sizeof(Result),
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    RETURN_ON_NULL_ERR(p, ERR_MMAP);
    error_slots = p;
    slot_count = count;
    return result_ok();
}

void reset_error_slots(int used) {
    if (used > slot_count) used = slot_count;
    memset(error_slots, 0, used * sizeof(Result));
}

void record_error(int slot, Result res) {
    if (slot < 0 || slot >= slot_count) return;
    error_slots[slot] = res;
}

void record_on_error(int slot, Result res) {
    if (is_error(res)) record_error(slot, res);
}

_Noreturn void record_on_err_and_exit(int slot, Result r) {
    record_on_error(slot, r);
    _exit(is_error(r) ? EXIT_FAILURE : r.exit_code);
}

void report_error(Result res) {
    if (!is_error(res)) return;

    ErrInfo info = err_info[res.type];
    bool has_text = false;
    if (res.context != NULL) {
        fprintf(stderr, "%s", res.context);
        has_text = true;
    }
    if (info.text != NULL) {
        fprintf(stderr, "%s%s", has_text ? ": " : "", info.text);
        has_text = true;
    }
    if (info.use_errno) {
        fprintf(stderr, "%s%s",
            has_text ? ": " : "",
            (res.type == ERR_SIGNALED) 
            ? strsignal(res.signal_code)
            : strerror(res.err_num)
        );
    }
    fputc('\n', stderr);
}

void report_all_errors(int used) {
    if (used > slot_count) used = slot_count;
    for (int i = 0; i < used; i++) {
        if (is_error(error_slots[i])) {
            report_error(error_slots[i]);
        }
    }
}

void report_syntax_err(const char* msg) {
    fprintf(stderr, "Syntax error: %s", msg);
}