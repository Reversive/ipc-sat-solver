#ifndef MASTER_H
#define MASTER_H
#include <stdio.h>
#include "../../include/error_codes.h"
#include "../communication/include/pipe.h"
#include <sys/wait.h>
#include <sys/select.h>
#define STDIN 0
#define STDOUT 1
#define STARTING_FILE_LIMIT 30
#define PATH_BUFFER_LIMIT 512
#define KILL_SIGNAL 9

void distribute_files(int * fda, char ** file_list, int file_count);
int summon_slaves(int file_count);
void kill_prev_slaves(int last_slave);
#endif