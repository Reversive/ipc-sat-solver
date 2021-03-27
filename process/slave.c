#include "include/slave.h"


int main(
    int argc, 
    char **argv) 
{
    
    char *path = NULL;
    size_t len = 0;
    ssize_t read;
    char payload[MAX_FILE_SIZE + NULL_TERMINATOR];
    while ((read = getline(&path, &len, stdin)) != EOF) {

        FILE *fp;
        fp = fopen(path,"r");
        if(fp == NULL)
        {
            perror("Unexpected problem opening file");
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
            payload[last_pos++] = '\0';
        }
        fclose(fp);
        printf("[CHILD %d] Retrieved path of length %zu : %s\n", getpid(), read, path);
        fflush(stdout);
    }
    free(path);
    return 0;
}

