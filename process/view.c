#include "view.h"

int main(
    int argc, 
    char **argv) 
{

    sleep(VIEW_SLEEP_INTERVAL);
    int shm_object = open_shared_mem_object(NAME, O_RDWR, 0);
    shm_buffer *shm_data = map_shared_memory(NULL, sizeof(*shm_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_object, 0);
    printf("\n[VIEW] Received:\n\n");
    wait_semaphore(&shm_data->bouncer);
    int size = shm_data->size[shm_data->read_position];
    char path_count[size];
    memcpy(path_count, shm_data->buffer[shm_data->read_position++], size);
    path_count[MAX_PATH_CHARS-1] = 0;
    int total = atoi(path_count);
    for(int i=0;i<total;i++)
    {
        char result[MAX_BUFFER_SIZE] = {0};
        wait_semaphore(&shm_data->bouncer);
        size = shm_data->size[shm_data->read_position];
        memcpy(result, shm_data->buffer[shm_data->read_position++], size);
        printf("%s\n", result);
    }

    printf("Finished\n");
}