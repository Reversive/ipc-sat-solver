#ifndef SHARED_MEM_H
#define SHARED_MEM_H
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../../include/error_codes.h"

#define NAME "/SO.TP1"

int open_shared_mem_object(const char *name,  int oflag, mode_t mode);
void extend_shared_mem(int fd, off_t length);
void* map_shared_memory(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
void unlink_shared_memory(const char *name);
#endif