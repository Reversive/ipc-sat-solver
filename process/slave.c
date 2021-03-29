#include "include/slave.h"

int main(
    int argc, 
    char **argv) 
{
    char *path = NULL;
    size_t len = 0;
    ssize_t read;
    char payload[MAX_FILE_SIZE + NULL_TERMINATOR];
    while ((read = getline(&path, &len, stdin)) != EOF) 
    {
        FILE *fp;
        fp = fopen(path, "r");
        if(fp == NULL)
        {
            perror(path);
            exit(EXIT_FAILURE);
        }

        size_t last_pos = fread(payload, sizeof(char), MAX_FILE_SIZE, fp);

        if(ferror( fp ) != 0 )
        {
            perror("Unexpected problem reading file");
            exit(EXIT_FAILURE);
        }
        else
        {
            payload[last_pos++] = '|';
        }

        fclose(fp);
        write_fd(STDOUT, payload, last_pos);
    }

    if(errno == SYS_FAILURE)
    {
        perror("Problem with getline()");
        exit(EXIT_FAILURE);
    }

    close_fd(STDOUT);
    free(path);
    return 0;
}

