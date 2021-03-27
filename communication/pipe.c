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
    int *pipe_array, 
    int size)
{
    for(int i = 0; i < size; i++)
    {
        open_pipe(&pipe_array[ PIPE_SIZE * i]);
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
    int * fda, 
    int idx, 
    char * payload,
    int size)
{
    write_fd(fda[PIPE_SIZE * idx + OUT], payload, size);
}

void close_fd(int fd)
{
    if( close(fd) == SYS_FAILURE )
    {
        perror("close() close_fd");
        exit(PIPE_CLOSE_FAILURE);
    }
}

void close_pipe(
    int * fd,
    int side,
    int size)
{

    for(int i = 0; i < size; i++)
    {
        switch (side)
        {
        case IN:
            close_fd(fd[PIPE_SIZE * i + IN]);
            break;
        case OUT:
            close_fd(fd[PIPE_SIZE * i + OUT]);
            break;
        
        default:
            perror("close_pipe() invalid side");
            exit(PIPE_CLOSE_FAILURE);
        }
    }
    
}


void redirect_pipe(
    int *fda,
    int slave_id,
    int from, 
    int to,
    int size)
{
    if(from != OUT && from != IN)
    {
        perror("bad pipe_redirect args");
        exit(PIPE_REDIRECT_FAILURE);
    }

    for(int i = 0; i < size; i++)
    {
        if(i == slave_id)
        {
            if( dup2(fda[PIPE_SIZE * i + from], to) == SYS_FAILURE )
            {
                perror("dup2()");
                exit(PIPE_REDIRECT_FAILURE);
            }
        }
        else
        {
            close_fd(fda[PIPE_SIZE * i + from]);
        }
    }
}



int set_fd_array(
    int * fda, 
    fd_set * fds,
    int slave_count)
{
    int i;
    int highest = 0;
    for(i = 0; i < slave_count; i++)
    {
        int current_fd = i * PIPE_SIZE;
        if(fda[current_fd] > highest)
        {
            highest = fda[current_fd];
        }
        FD_SET(fda[current_fd], fds);
    }
    return highest;
}

