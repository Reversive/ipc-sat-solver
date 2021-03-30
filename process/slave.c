#include "include/slave.h"

int main(
    int argc, 
    char **argv) 
{
    char *path = NULL;
    size_t len = 0;
    ssize_t read;
    errno = 0;
    while ((read = getline(&path, &len, stdin)) != EOF) 
    {
        char command_args[1024];
        strcat(command_args,"minisat ");
        strcat(command_args,path);
        strcat(command_args," | grep -o -e 'Number of.*[0-9]\\+' -e 'CPU time.*' -e '.*SATISFIABLE'");

        FILE * fp = popen(command_args, "r");
        if (fp == NULL) {
            perror("Failed popen");
            exit(EXIT_FAILURE);
        }
        
        char payload[MAX_FILE_SIZE + NULL_TERMINATOR];

        rewind(fp);

        size_t last_pos = fread(payload, sizeof(char), MAX_FILE_SIZE, fp);

        if(ferror( fp ) != 0 ) {
            perror("Unexpected problem reading file");
            exit(EXIT_FAILURE);
        }else
        {
            payload[last_pos] = '*';
        }

        pclose(fp);
        char tmp[20];
        sprintf(tmp,"%d\n",last_pos);
        write_fd(2, tmp, strlen(tmp)+1);
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

