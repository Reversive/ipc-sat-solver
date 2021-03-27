#ifndef MASTER_H
#define MASTER_H
#include <stdio.h>
#include "../../include/error_codes.h"
#include "../../communication/include/pipe.h"
#include <sys/wait.h>
#include <sys/select.h>

#define QUEUE_START 0
#define QUEUE_EMPTY 0
#define STDIN 0
#define STDOUT 1
#define STARTING_FILE_LIMIT 30
#define PATH_BUFFER_LIMIT 512
#define KILL_SIGNAL 9
#define MAX_SLAVE_COUNT 15
#define PIPE_ARRAY_SIZE PIPE_SIZE * MAX_SLAVE_COUNT 


void distribute_files(int * fda, char ** file_list, int file_count, int slave_count);
void summon_slaves(int slave_count);
void kill_previous_slaves(int last_slave);
int check_fd_array_set(int * fda, fd_set * fds, int slave_count);
#endif