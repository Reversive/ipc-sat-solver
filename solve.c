#include "include/solve.h"

int main(
    int argc, 
    char **argv) 
{
    if(argc == NO_ARGS)
    {
        printf("No file was provided to solve.\n");
        exit(BAD_ARG_COUNT);
    }

    int file_count = argc - FIRST_ARG;
    
    
    return 0;
}