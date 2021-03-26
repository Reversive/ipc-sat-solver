#include "include/master.h"

int master_fd[PIPE_ARRAY_SIZE];
int slave_fd[PIPE_ARRAY_SIZE];
int slave_pid[MAX_SLAVE_COUNT];
char *file_buffer[PATH_BUFFER_LIMIT];

int main(
    int argc, 
    char **argv) 
{
    
    int file_count = argc;
    char **file_names = argv;
    open_pipe_array(master_fd, MAX_SLAVE_COUNT);
    open_pipe_array(slave_fd, MAX_SLAVE_COUNT);
    distribute_files(master_fd, file_names, file_count);
    close_pipe_array_side(master_fd, WRITE);
    int slave_count = summon_slaves(file_count);

    for(;;)
    {
        fd_set fds;        
        FD_ZERO(&fds);
        int maxfd = set_fd_array(slave_fd, &fds, slave_count) + 1;
        if( select(maxfd, &fds, NULL, NULL, NULL) == SYS_FAILURE )
        {
            perror("select()");
            exit(SELECT_FAILURE);
        }
        check_fd_array_set(slave_fd, &fds, slave_count);
        sleep(1);
    }
    
    for(int i = 0; i < slave_count; i++)
    {
        int fr;
        waitpid(slave_pid[i], &fr, 0);
    }

}

void distribute_files(
    int * fda, 
    char ** file_list, 
    int file_count)
{
    int i;
    for(i = 0; i < file_count && i != STARTING_FILE_LIMIT; i++)
    {
        if( write_pipe_by_index(fda, i % MAX_SLAVE_COUNT, file_list[i]) == SYS_FAILURE)
        {
            perror("write()");
            exit(PIPE_WRITE_FAILURE);
        }
    }

    if(i == STARTING_FILE_LIMIT)
    {
        for(int j = i; j < file_count; j++)
        {
            file_buffer[j - STARTING_FILE_LIMIT] = file_list[j];
        }
    }

}

int summon_slaves(
    int file_count)
{
    int slave_count = file_count >= MAX_SLAVE_COUNT ? MAX_SLAVE_COUNT : file_count;
    pid_t pid;
    char *argv[] = { NULL };
    for(int i = 0; i < slave_count; i++)
    {
        pid = fork();

        if(pid == SYS_FAILURE)
        {
            perror("fork()");
            kill_prev_slaves(i);
            exit(SLAVE_FORK_FAILURE);
        }

        if(pid == 0)
        {
            close_pipe_array_side(slave_fd, READ);
            redirect_pipe_array(master_fd, i, READ, STDIN);
            redirect_pipe_array(slave_fd, i, WRITE, STDOUT);
            execv("./process/bin/slave", argv);
            perror("execv()");
            exit(EXECV_FAILURE);
        }

        slave_pid[i] = pid;
    }
    return slave_count;
}


void kill_prev_slaves(int last_slave)
{
    int i;
    for(i = 0; i < last_slave; i++)
    {
        kill(slave_pid[i], KILL_SIGNAL);
    }
}