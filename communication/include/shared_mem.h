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
#include "../../include/utils.h"
#include <errno.h>
#define EEXIST 17
#define NAME "/SO.TP1"
#define PROCESS_SHARED 1

typedef struct {
    sem_t  bouncer;
    int write_position;
    int read_position;
    char buffer[PATH_BUFFER_LIMIT][MAX_BUFFER_SIZE];
    int size[PATH_BUFFER_LIMIT];
} shm_buffer;

int open_shared_mem_object(const char *name,  int oflag, mode_t mode);
void init_shared_mem_data(shm_buffer *data, int value);
void extend_shared_mem(int fd, off_t length);
void* map_shared_memory(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
void unmap_shared_memory(void* addr, size_t len);
void unlink_shared_memory(const char *name);
void init_semaphore(sem_t *semaphore, int pshared, unsigned int value);
void wait_semaphore(sem_t *semaphore);
void post_semaphore(sem_t *semaphore);


#endif