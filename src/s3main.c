#include "s3.h"
#include "pipes.h"
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

        if (!handle_pipeline(line)) {
            parse_command(line, args, &argsc);
            
            if (args[0] == NULL) {
                continue;
            }

            if (is_cd(args)) {
                run_cd(args, argsc, lwd); 
            }
            else launch_program(args, argsc);
        }      
        reap();
    }

    return EXIT_SUCCESS;
}