#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define OOM_ERR_MSG "Arena allocator ran out memory"

#define RETURN_IF_ERR(expr)                      \
    do {                                         \
        Result _res = (expr);                    \
        if (_res.type != ERR_NONE) return _res; \
    } while (0)

#define GOTO_IF_ERR(expr)                      \
    do {                                         \
        Result _res = (expr);                    \
        if (_res.status != STATUS_OK) goto cleanup; \
    } while (0)

#define CLEANUP_ON_SYS_ERR(rc, res, _type) \
    do {                                \
        if (rc >= 0) continue;          \
        res = result_sys((_type));                  \
        goto cleanup; \
    } while (0)


#define RETURN_ON_SYS_ERR(rc, _type) \
    do {                                    \
        if ((rc) < 0) return result_sys((_type)); \
    } while(0) \

#define RETURN_ON_NULL_ERR(ptr, _type) \
    do {                                    \
        if ((ptr) == NULL) return result_sys((_type)); \
    } while(0) \

#define RETURN_WITH_CONTEXT_ON_SYS_ERR(rc, _type, _context) \
    do {                    \
        if ((rc) >= 0) break; \
        Result res = result_sys((_type)); \
        res.context = (_context); \
        return res; \
    } while(0) \

typedef enum {
    ERR_NONE = 0,
    ERR_FORK,
    ERR_PIPE,
    ERR_DUP2,
    ERR_OPEN,
    ERR_CLOSE,
    ERR_WAIT,
    ERR_CD,
    ERR_MMAP,
    ERR_EXEC,
    ERR_SIGNALED,
    ERR_COUNT 
} ErrCode;

typedef struct {
    uint8_t type;
    union {
        uint8_t exit_code; 
        uint8_t err_num; 
        uint8_t signal_code;
    };
    const char* context;
} Result;

typedef struct {
    const char* text;
    bool use_errno;
} ErrInfo;

extern const ErrInfo err_info[ERR_COUNT];

static inline Result result_ok() { return (Result){ .type=ERR_NONE, .exit_code = 0, .context=NULL }; }
static inline Result result_exit(int exit_code) {
    return (Result){ .type = ERR_NONE, .exit_code = (uint8_t)exit_code };
}
static inline bool is_error(Result result) { return result.type != ERR_NONE; }
Result result_sys(ErrCode code);
Result get_child_result(int status);

Result error_slots_init(int count);
void reset_error_slots(int used);
void record_error(int slot, Result res);
_Noreturn void record_on_err_and_exit(int slot, Result r);
void record_on_error(int slot, Result r);
void report_all_errors(int used);
void report_error(Result res);
void report_syntax_err(const char* msg);