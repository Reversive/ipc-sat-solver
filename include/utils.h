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

int is_valid_file(const char *path);
int is_valid_batch(char **paths, int len);


#endif