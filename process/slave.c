#include "include/slave.h"


int main(
    int argc, 
    char **argv) 
{
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, stdin)) != -1) {
        printf("[CHILD %d] Retrieved line of length %zu : %s\n", getpid(), read, line);
        fflush(stdout);
    }
    free(line);
    return 0;
}

