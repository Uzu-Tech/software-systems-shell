#include "ast.h"

typedef enum { EXEC_IN_PLACE, EXEC_FORK } ExecMode;

typedef enum {
    CMD_NORMAL,
    CMD_CD,
    CMD_EXIT
} CommandType;

Result execute_node(ASTNode* node, int in_fd, int out_fd, ExecMode mode);