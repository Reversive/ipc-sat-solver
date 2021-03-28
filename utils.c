#include "include/utils.h"


int is_valid_file(
    const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}


int is_valid_batch(
    char **paths, 
    int len)
{
    int result = 0;
    for(int i = 0; i < len; i++)
    {
        result += is_valid_file(paths[i]);
    }

    return result == len ? SUCCESS : FAILURE;
}


void init_container_array(
    slave_container * container,
    int size)
{
    for(int i = 0; i < size; i++)
    {
        container[i].pos = 0;
        for(int j = 0; j < MAX_BUFFER_SIZE; j++)
        {
            container[i].buffer[j] = 0;
        }
    }
}


void reset_container(
    slave_container * container,
    int idx)
{
    container[idx].pos = 0;
    for(int i = 0; i < MAX_BUFFER_SIZE; i++)
    {
        container[idx].buffer[i] = 0;
    }
}