#include "include/pipe.h"

void open_pipe_array(
    int * fda, 
    int count)
{
    int i;
    for(i = 0; i < count; i++)
    {
        if( pipe(&fda[ PIPE_SIZE * i]) == SYS_FAILURE )
        {
            perror("pipe()");
            exit(PIPE_FAILURE);
        }
    }
}

ssize_t write_pipe_by_index(
    int * fda, 
    int idx, 
    const char * payload)
{
    unsigned char payload_size = (char)(strlen(payload) + NULL_TERMINATOR);
    char payload_package_size[1] = { payload_size };
    if( write(fda[PIPE_SIZE * idx + WRITE], payload_package_size, BYTE) == SYS_FAILURE)
    {
        perror("write() pipe_by_index");
        exit(PIPE_WRITE_FAILURE);
    }
    return write(fda[PIPE_SIZE * idx + WRITE], payload, strlen(payload) + NULL_TERMINATOR);
}


void close_pipe_array_side(
    int * fda, 
    int side)
{
    int i;
    for(i = 0; i < PIPE_ARRAY_SIZE / PIPE_SIZE; i++)
    {
        if(side == WRITE)
        {
            if( close(fda[PIPE_SIZE * i + 1]) == SYS_FAILURE )
            {
                perror("close() write");
                exit(PIPE_CLOSE_FAILURE);
            }
        }
        else if(side == READ) 
        {
            if( close(fda[PIPE_SIZE * i]) == SYS_FAILURE )
            {
                perror("close() read");
                exit(PIPE_CLOSE_FAILURE);
            }
        }
        else
        {
            perror("close() bad arg");
            exit(PIPE_CLOSE_FAILURE);
        }
    }
}


void redirect_pipe_array(
    int *fda,
    int slave_id,
    int from, 
    int to)
{
    if(from != WRITE && from != READ)
    {
        perror("bad pipe_redirect args");
        exit(PIPE_REDIRECT_FAILURE);
    }

    for(int i = 0; i < MAX_SLAVE_COUNT; i++)
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
            if( close(fda[PIPE_SIZE * i + from]) == SYS_FAILURE )
            {
                perror("close() redirect");
                exit(PIPE_CLOSE_FAILURE);
            }
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

void check_fd_array_set(
    int * fda, 
    fd_set * fds,
    int slave_count)
{
    int i;
    char *payload = NULL;
    for(i = 0; i < slave_count; i++)
    {
        int fd = fda[i * PIPE_SIZE];
        
        if(FD_ISSET(fd, fds))
        {
            char payload_size;
            int payload_size_bytes_read = read(fd, &payload_size, BYTE);
            int payload_bytes_read = 0;
            if( payload_size_bytes_read == SYS_FAILURE )
            {
                perror("payload_size - read()");
                exit(PIPE_FAILURE);
            }
            else if(payload_size_bytes_read == 1)
            {
                payload = malloc(payload_size);
                do
                {
                    payload_bytes_read = read(fd, payload, payload_size);
                    if( payload_bytes_read == SYS_FAILURE )
                    {
                        perror("payload - read()");
                        exit(PIPE_FAILURE);
                    }

                } while (payload_bytes_read != payload_size);

                printf("Read %d bytes containing: %s\n", payload_size, payload);
            }
            
        }
    }
}  