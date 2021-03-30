#include "include/pipe.h"


void open_pipe(int *m_pipe)
{
    if( pipe(m_pipe) == SYS_FAILURE )
    {
        perror("pipe() open_pipe");
        exit(PIPE_FAILURE);
    }
}

void open_pipe_array(
    int *m_pipe, 
    int size)
{
    int current_pipe;
    for(current_pipe = 0; current_pipe < size; current_pipe++)
    {
        open_pipe(&m_pipe[ PIPE_SIZE * current_pipe]);
    }
}

void write_fd(
    int fd, 
    char *message, 
    int size)
{
    if( write(fd, message, size) == SYS_FAILURE)
    {
        perror("write() write_fd");
        exit(PIPE_WRITE_FAILURE);
    }
}

void write_pipe(
    int * m_pipe, 
    int idx, 
    char * payload,
    int size)
{
    write_fd(m_pipe[PIPE_SIZE * idx + WRITE], payload, size);
}

void close_fd(int fd)
{
    if( close(fd) == SYS_FAILURE )
    {
        perror("close() close_fd");
        exit(PIPE_CLOSE_FAILURE);
    }
}

void close_pipe_array(
    int *m_pipe,
    int side,
    int size)
{
    int current_pipe;
    for(current_pipe = 0; current_pipe < size; current_pipe++)
    {
        switch (side)
        {
        case READ:
            close_fd(m_pipe[PIPE_SIZE * current_pipe + READ]);
            break;
        case WRITE:
            close_fd(m_pipe[PIPE_SIZE * current_pipe + WRITE]);
            break;
        
        default:
            perror("close_pipe() invalid side");
            exit(PIPE_CLOSE_FAILURE);
        }
    }
    
}


void redirect_pipe(
    int *m_pipe,
    int slave_id,
    int from, 
    int to,
    int size)
{
    if(from != WRITE && from != READ)
    {
        perror("bad pipe_redirect args");
        exit(PIPE_REDIRECT_FAILURE);
    }
    if( dup2(m_pipe[PIPE_SIZE * slave_id + from], to) == SYS_FAILURE )
    {
        perror("dup2()");
        exit(PIPE_REDIRECT_FAILURE);
    }
    
}



int set_pipe_array(
    int *fdr_array,  
    fd_set *fdr,
    int slave_count)
{
    int highest = 0;
    int current_pipe;
    for(current_pipe = 0; current_pipe < slave_count; current_pipe++)
    {
        int current_rfd = current_pipe * PIPE_SIZE;
        if(fdr_array[current_rfd] > highest)
        {
            highest = fdr_array[current_rfd];
        }

        FD_SET(fdr_array[current_rfd], fdr);
    }
    return highest + 1;
}

