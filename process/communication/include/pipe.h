#ifndef PIPE_H
#define PIPE_H
#include <unistd.h>
#include "../../../include/error_codes.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>  
#include <sys/select.h>

#define PIPE_SIZE 2
#define NULL_TERMINATOR 1
#define BYTE 1
#define WRITE 1
#define READ 0
#define MAX_SLAVE_COUNT 15
#define PIPE_ARRAY_SIZE PIPE_SIZE * MAX_SLAVE_COUNT 

void open_pipe_array(int * fda, int count);
ssize_t write_pipe_by_index(int * fda, int idx, const char * str);
void close_pipe_array_side(int * fda, int side);
void redirect_pipe_array(int *fda, int slave_id, int from, int to);
int set_fd_array(int * fda, fd_set * fds, int slave_count);
void check_fd_array_set(int * fda, fd_set * fds, int slave_count);
#endif