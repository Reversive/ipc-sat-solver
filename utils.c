#include "include/utils.h"


int is_valid_file(const char *path)
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