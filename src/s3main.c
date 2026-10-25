#include "s3.h"
#include "pipes.h"
#include "batch.h"
#include "cd.h"

int main(int argc, char *argv[]){
    ///Stores the command line input
    char line[MAX_LINE];
    char *args[MAX_ARGS];
    char lwd[MAX_LINE] = "";

    ///Stores the number of arguments
    int argsc;

    for (;;) {
        read_command_line(line);

        if (line[0] == '\0') {
            continue;
        }

        handle_batch(line, lwd);
        reap();
    }

    return EXIT_SUCCESS;
}