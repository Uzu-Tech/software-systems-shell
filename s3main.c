#include "s3.h"

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];
    char *args[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;

    while (1) {
        read_command_line(line);

        parse_command(line, args, &argsc);

        launch_program(args, argsc); 

        reap();
    }

    return 0;
    
}