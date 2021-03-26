#include "include/solve.h"

int main(
    int argc, 
    char **argv) 
{
    if(argc == NO_ARGS)
    {
        printf("No file was provided to solve.\nTerminating Program...\n");
        exit(BAD_ARG_COUNT);
    }

    int file_count = argc - FIRST_ARG;
    
    if(file_count > MAX_ARG_COUNT)
    {
        printf("Can't process more than 1024 files.\nTerminating Program...\n");
        exit(BAD_ARG_FILE);
    }

    char **n_argv = argv + FIRST_ARG;
    if(is_valid_batch(n_argv, file_count))
    {
        execv("./process/bin/master", n_argv);
        perror("execve");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("One or more arguments are not valid files.\nTerminating Program...\n");
        exit(BAD_ARG_FILE);
    }
    
    return 0;
}