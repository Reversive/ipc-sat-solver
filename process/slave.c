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
        remove_char(path, '\n');
        char command_args[MAX_PATH_SIZE] = {0};
        strcat(command_args,"minisat ");
        strcat(command_args, path);
        strcat(command_args," | grep -o -e 'Number of.*[0-9]\\+' -e 'CPU time.*' -e '.*SATISFIABLE' | grep -o -e '[0-9|.]*' -o -e '.*SATISFIABLE' | xargs | sed 's/ /\\t/g'");
        
        FILE * fp = popen(command_args, "r");
        if (fp == NULL) {
            perror("Failed popen");
            exit(EXIT_FAILURE);
        }
        
        char payload[MAX_FILE_SIZE] = {0};

        size_t last_pos = fread(payload, sizeof(char), MAX_FILE_SIZE, fp);
        remove_char(payload, '\n');
        if(ferror( fp ) != 0 ) {
            perror("Unexpected problem reading file");
            exit(EXIT_FAILURE);
        }
        else
        {
            strcat(payload, "*");
        }
        
        pclose(fp);

        int pid = getpid();
        char results[MAX_BUFFER_SIZE];
        int num_var, num_clauses;
        float cpu_time;
        char sat[SAT_SIZE]={0};
        sscanf(payload, "%d\t%d\t%f\t%s", &num_var, &num_clauses, &cpu_time, sat);
        sat[strlen(sat)-1] = 0;
        sprintf(results, "File: %s\nPID: %d\nNumber of variables: %d\nNumber of clauses: %d\nCPU Time: %f\n%s\n\n", path, pid, num_var, num_clauses, cpu_time, sat);
        
        write_fd(STDOUT, results, strlen(results));
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



