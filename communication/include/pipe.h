#ifndef PIPE_H
#define PIPE_H
#include <unistd.h>
#include "../../include/error_codes.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>  
#include <sys/select.h>

#define PIPE_SIZE 2
#define NULL_TERMINATOR 1
#define BYTE 1
#define WRITE 1
#define READ 0

void open_pipe(int *m_pipe);
void open_pipe_array(int *m_pipe, int size);
void write_fd(int fd, char *message, int size);
void write_pipe(int *m_pipe, int idx, char *payload, int size);
void close_fd(int fd);
void close_pipe_array(int *m_pipe, int side, int size);
void redirect_pipe(int *m_pipe, int slave_id, int from, int to, int size);
int set_pipe_array(int *fd_array, fd_set *fdr, int slave_count);

#endif