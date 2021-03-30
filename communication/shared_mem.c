#include "include/shared_mem.h"


int open_shared_mem_object(
    const char *name, 
    int oflag, 
    mode_t mode)
{
    int fd = shm_open(NAME, oflag, mode);
    if(fd == SYS_FAILURE)
    {
        perror("shm_open()");
        exit(SHM_OPEN_FAILURE);
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


void unlink_shared_memory(
    const char *name)
{
    if(shm_unlink(name) == SYS_FAILURE)
    {
        perror("shm_unlink()");
        exit(UNLINK_FAILURE);
    }
}