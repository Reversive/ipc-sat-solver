#include "include/master.h"

int master_fd[PIPE_ARRAY_SIZE];
int slave_fd[PIPE_ARRAY_SIZE];
int slave_pid[MAX_SLAVE_COUNT];
char *file_buffer[PATH_BUFFER_LIMIT];
int queue_pos = QUEUE_START;
int queue_size = QUEUE_EMPTY;

int main(
    int argc, 
    char **argv) 
{
    setbuf(stdout, NULL);
    int file_count = argc;
    char **file_names = argv;
    int slave_count = file_count >= STARTING_FILE_LIMIT ? MAX_SLAVE_COUNT : file_count;
    open_pipe_array(master_fd, slave_count);
    open_pipe_array(slave_fd, slave_count);
    distribute_files(master_fd, file_names, file_count, slave_count);
    summon_slaves(slave_count);
    sleep(3);
    close_pipe(master_fd, OUT, slave_count);

    for(int i = 0; i < slave_count; i++)
    {
        int fr;
        waitpid(slave_pid[i], &fr, 0);
    }

}

void distribute_files(
    int * fda, 
    char ** file_list, 
    int file_count,
    int slave_count)
{
    int i;
    for(i = 0; i < file_count && i != STARTING_FILE_LIMIT; i++)
    {
        write_pipe(fda, i % slave_count, file_list[i], strlen(file_list[i]) + NULL_TERMINATOR);
        write_pipe(fda, i % slave_count, "\n", BYTE);
    }

    if(i == STARTING_FILE_LIMIT)
    {
        for(int j = i; j < file_count; j++)
        {
            file_buffer[queue_size++] = file_list[j];
        }
    }
}

void summon_slaves(
    int slave_count)
{
    pid_t pid;
    char *argv[] = { NULL };
    for(int i = 0; i < slave_count; i++)
    {
        pid = fork();

        if(pid == SYS_FAILURE)
        {
            perror("fork()");
            kill_previous_slaves(i);
            exit(SLAVE_FORK_FAILURE);
        }

        if(pid == 0)
        {
            close_pipe(slave_fd, IN, slave_count);
            redirect_pipe(master_fd, i, IN, STDIN, slave_count);
            //redirect_pipe(slave_fd, i, OUT, STDOUT, slave_count);
            execv("./process/bin/slave", argv);
            perror("execv()");
            exit(EXECV_FAILURE);
        }

        slave_pid[i] = pid;
    }
}


void kill_previous_slaves(int last_slave)
{
    int i;
    for(i = 0; i < last_slave; i++)
    {
        kill(slave_pid[i], KILL_SIGNAL);
    }
}

