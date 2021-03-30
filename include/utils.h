#ifndef UTILS_H
#define UTILS_H
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "error_codes.h"

#define NO_ARGS 1
#define FIRST_ARG 1
#define SUCCESS 1
#define FAILURE 0
#define MAX_BUFFER_SIZE 16384
#define PATH_BUFFER_LIMIT 1024
#define QUEUE_START 0
#define QUEUE_EMPTY 0

enum STATUS { STOP = 0, RUNNING };

typedef struct {
    int pos;
    char buffer[MAX_BUFFER_SIZE];
} slave_container;

typedef struct {
    char *file_buffer[PATH_BUFFER_LIMIT];
    int queue_pos;
    int queue_size;
    int queue_read;
} file_container;

int is_valid_file(const char *path);
int is_valid_batch(char **paths, int len);
void init_container_array(slave_container * container, int size);
void reset_container(slave_container * container, int idx);
void remove_char(char *s, char chr);
#endif