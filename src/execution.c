#include "execution.h"
#include "ast.h"
#include "child.h"
#include "error.h"
#include "child.h"
#include "lexer.h"
#include "pipes.h"
#include "cd.h"
#include "shell.h"
#include "utils.h"
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

static bool enter_child(ExecMode mode, int id, Result* res) {
    if (mode == EXEC_IN_PLACE) return true;

    pid_t rc = fork();
    if (rc < 0) {
        *res = result_sys(ERR_FORK);
        return false;
    }

    if (is_child(rc)) return true;

    // Parent Path
    *res = reap_child(rc);
    return false;
}

static CommandType get_command_type(char** argv) {
    if (is_cd(argv)) {
        return CMD_CD;
    }
    return CMD_NORMAL;
}

static Result execute_special_command(CommandType cmd_type, char** argv) {
    switch (cmd_type) {
        case CMD_CD:
            return run_cd(argv, lwd); // lwd is global
        default:
            return result_ok();
    }
}

Result execute_command(Command* command, int in_fd, int out_fd, int id, ExecMode mode) {
    CommandType cmd_type = get_command_type(command->argv);
    if (cmd_type != CMD_NORMAL) {
        return execute_special_command(cmd_type, command->argv);
    }

    Result res;
    if (!enter_child(mode, id, &res)) return res;

    // Child Path
    ChildParams params  = {
        .args = command->argv,
        .in_fd = in_fd,
        .out_fd = out_fd
    };

    Result child_res = exec_program(&params);
    // Only reaches this code on error when launching new program
    record_on_err_and_exit(id, child_res);
}

Result execute_subshell(Subshell* subshell, int in_fd, int out_fd, int id, ExecMode mode) {
    Result parent_res;
    if (!enter_child(mode, id, &parent_res)) return parent_res;

    // Child path
    Result res = execute_node(subshell->child, in_fd, out_fd, mode);
    record_on_err_and_exit(id, res);
}

Result execute_redirect(Redirect* redirect, int in_fd, int out_fd, ExecMode mode) {
    static int flags[] = {O_RDONLY, O_CREAT | O_WRONLY | O_TRUNC, O_CREAT | O_WRONLY | O_APPEND };
    
    int fd = (redirect->redir_type == TOKEN_REDIR_IN)
        ? open(redirect->file, flags[redirect->redir_type])
        : open(redirect->file, flags[redirect->redir_type], 0666);
    RETURN_WITH_CONTEXT_ON_SYS_ERR(fd, ERR_OPEN, redirect->file);

    Result res = (redirect->redir_type == TOKEN_REDIR_IN)
        ? execute_node(redirect->child, fd, out_fd, mode)
        : execute_node(redirect->child, in_fd, fd, mode);

    int target_fd = redirect->redir_type == TOKEN_REDIR_IN
        ? STDIN_FILENO
        : STDOUT_FILENO;
    
    if (fd != target_fd && close(fd) < 0 && !is_error(res)) {
        return result_sys(ERR_CLOSE);
    }

    return res;
}

static _Noreturn void exec_pipeline_stage(ASTNode* child, Pipe pipes[], int pipe_count, int in_fd, int out_fd) {
    Result res = redirect_stdio(in_fd, out_fd);
    if (is_error(res)) record_on_err_and_exit(child->id, res);

    close_pipeline_pipes(pipes, pipe_count);

    res = execute_node(child, STDIN_FILENO, STDOUT_FILENO, EXEC_IN_PLACE);
    record_on_err_and_exit(child->id, res);
}

Result execute_pipeline(Pipeline* pipeline, int in_fd, int out_fd) {
    int pipe_count = pipeline->count - 1;
    Pipe pipes[MAX_COMMANDS];
    Result res = result_ok();

    RETURN_IF_ERR(create_pipeline_pipes(pipes, pipe_count));

    pid_t pids[MAX_COMMANDS];
    int num_forked = 0;

    for (int i = 0; i < pipeline->count; i++) {
        int pipe_fd_in = (i == 0) ? in_fd : pipes[i - 1].read;
        int pipe_fd_out = (i == pipeline->count - 1) ? out_fd : pipes[i].write;

        pid_t rc = fork();
        CLEANUP_ON_SYS_ERR(rc, res, ERR_FORK);

        if (is_child(rc)) {
            exec_pipeline_stage(pipeline->children[i], pipes, pipe_count, pipe_fd_in, pipe_fd_out);
        }

        pids[num_forked++] = rc;
    }

cleanup:
    close_pipeline_pipes(pipes, pipe_count);

    for (int i = 0; i < num_forked; i++) {
        Result r = reap_child(pids[i]);
        if (is_error(r) && !(r.type == ERR_SIGNALED && r.signal_code == SIGPIPE)) {
            record_error(pipeline->children[i]->id, r);
        }
    }

    return res;
}


Result execute_binary_op(BinaryOp* op, int in_fd, int out_fd) {
    switch (op->op_type) {
        case TOKEN_AND:
            {
                Result left = execute_node(op->left, in_fd, out_fd, EXEC_FORK);
                if (is_error(left) || left.exit_code != 0) return left;
            }
            return execute_node(op->right, in_fd, out_fd, EXEC_FORK);

        case TOKEN_OR:
            Result res = execute_node(op->left, in_fd, out_fd, EXEC_FORK);
            if (!is_error(res) && res.exit_code == 0) {
                return res;
            }
            if (is_error(res)) record_on_error(op->left->id, res);
            return execute_node(op->right, in_fd, out_fd, EXEC_FORK);
        
        default:
            return result_ok();
    }
}

Result execute_batch(Batch* batch, int in_fd, int out_fd) {
    for (int i = 0; i < batch->count; i++) {
        Result res = execute_node(batch->children[i], in_fd, out_fd, EXEC_FORK);
        record_on_error(batch->children[i]->id, res);
    }
    return result_ok();
}


Result execute_node(ASTNode* node, int in_fd, int out_fd, ExecMode mode) {
    switch (node->type) {
        case AST_COMMAND:
            return execute_command(&node->command, in_fd, out_fd, node->id, mode);
        case AST_SUBSHELL:
            return execute_subshell(&node->subshell, in_fd, out_fd, node->id, mode);
        case AST_REDIRECT:
            return execute_redirect(&node->redirect, in_fd, out_fd, mode);
        case AST_PIPELINE:
            return execute_pipeline(&node->pipeline, in_fd, out_fd);
        case AST_BINARY_OP:
            return execute_binary_op(&node->bin_op, in_fd, out_fd);
        case AST_BATCH:
            return execute_batch(&node->batch, in_fd, out_fd);
        default:
            return result_ok();
    }
}
