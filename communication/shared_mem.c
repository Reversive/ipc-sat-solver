#include "include/shared_mem.h"


void init_shared_mem_data(
    shm_buffer *data, 
    int value)
{
    data->write_position = value;
    data->read_position = value;
    for(int i = 0; i < PATH_BUFFER_LIMIT; i++)
    {
        for(int j = 0; j < MAX_BUFFER_SIZE; j++)
        {
            data->buffer[i][j] = value;
        }
        data->size[i] = value;
    }
}

int open_shared_mem_object(
    const char *name, 
    int oflag, 
    mode_t mode)
{
    int fd = shm_open(NAME, oflag, mode);
    if(fd == SYS_FAILURE)
    {
        if(errno == EEXIST)
        {
            unlink_shared_memory(NAME);
            fd = shm_open(NAME, oflag, mode);
        }
        else
        {
            perror("shm_open()");
            exit(SHM_OPEN_FAILURE);
        }
    }
    return fd;
}


void extend_shared_mem(
    int fd, 
    off_t length)
{
    if(ftruncate(fd, length) == SYS_FAILURE)
    {
        perror("ftruncate()");
        exit(FTRUNCATE_FAILURE);
    }
}

void* map_shared_memory(
    void *addr, 
    size_t length, 
    int prot, 
    int flags, 
    int fd, 
    off_t offset)
{
    void *ret = mmap(addr, length, prot, flags, fd, offset);
    if(ret == MAP_FAILED)
    {
        perror("mmap()");
        exit(MMAP_FAILURE);
    }

    return ret;
}

void unmap_shared_memory(
    void* addr,
    size_t len)
{
    if(munmap(addr, len) == SYS_FAILURE)
    {
        perror("munmap()");
        exit(MMAP_FAILURE);
    }
}


void unlink_shared_memory(
    const char *name)
{
    if(shm_unlink(name) == SYS_FAILURE)
    {
        perror("shm_unlink()");
        exit(UNLINK_FAILURE);
    }
}

void init_semaphore(
    sem_t *semaphore, 
    int pshared, 
    unsigned int value)
{
    if(sem_init(semaphore, pshared, value) == SYS_FAILURE)
    {
        perror("sem_init()");
        exit(SEMAPHORE_FAILURE);
    }
}


void wait_semaphore(
    sem_t *semaphore)
{
    if(sem_wait(semaphore) == SYS_FAILURE)
    {
        perror("sem_wait()");
        exit(SEMAPHORE_FAILURE);
    }
}

void post_semaphore(
    sem_t *semaphore)
{
    if(sem_post(semaphore) == SYS_FAILURE)
    {
        perror("sem_post()");
        exit(SEMAPHORE_FAILURE);
    }
}