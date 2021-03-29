#ifndef MASTER_H
#define MASTER_H
#define  _GNU_SOURCE
#include <stdio.h>
#include "../../communication/include/pipe.h"
#include <sys/wait.h>
#include "../../include/utils.h"
#include <signal.h>
#include <sys/socket.h>

#define DELIMITER 1
#define NONE 0

#define STDIN 0
#define STDOUT 1

#define STARTING_FILE_LIMIT 30
#define CHUNK_SIZE 100

#define KILL_SIGNAL 9

#define MAX_SLAVE_COUNT 15
#define PIPE_ARRAY_SIZE PIPE_SIZE * MAX_SLAVE_COUNT 

#define VIEW_SLEEP_INTERVAL 2


void distribute_and_cache_paths(int * pipes, char ** path_list, int path_count, int slave_count);
void summon_slaves(int slave_count);
void kill_previous_slaves(int last_slave);
int check_fd_array_set(int * pipes, fd_set * fds, int slave_count);
void queue_next_path(int fd);
void poll_queue_next_path(int * is_pipe_closed, int write_fd, int slave_count);
void fix_internal_buffer(int idx, slave_container * sc, char * delim_offset);
void set_slaves_status(int * slave_status, int slave_count, enum STATUS status);
int get_running_slaves_count(int * slave_status, int slave_count);
void write_buffer_to_file(char * path, char * flag, int size, slave_container * sc, int current_slave);
#endif