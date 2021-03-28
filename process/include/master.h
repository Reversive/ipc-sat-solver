#ifndef MASTER_H
#define MASTER_H
#define  _GNU_SOURCE
#include <stdio.h>
#include "../../communication/include/pipe.h"
#include <sys/wait.h>
#include "../../include/utils.h"
#include <signal.h>
#include <sys/socket.h>

#define TIMEOUT 0
#define DELIMITER 1
#define ZERO 0

#define STDIN 0
#define STDOUT 1

#define STARTING_FILE_LIMIT 30
#define CHUNK_SIZE 100

#define KILL_SIGNAL 9

#define MAX_SLAVE_COUNT 15
#define PIPE_ARRAY_SIZE PIPE_SIZE * MAX_SLAVE_COUNT 

#define VIEW_SLEEP_INTERVAL 2


void distribute_files(int * fda, char ** file_list, int file_count, int slave_count);
void summon_slaves(int slave_count);
void kill_previous_slaves(int last_slave);
int check_fd_array_set(int * fda, fd_set * fds, int slave_count);
void queue_next_file(int fd);
void fix_internal_buffer(int idx, slave_container * sc, char * delim_offset);

#endif