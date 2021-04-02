#include "view.h"

int main(
    int argc, 
    char **argv) 
{

    sleep(VIEW_SLEEP_INTERVAL);
    int shm_object = open_shared_mem_object(NAME, O_RDWR, 0);
    shm_buffer *shm_data = map_shared_memory(NULL, sizeof(*shm_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_object, 0);
    int status = RUNNING;
    while(status == RUNNING)
    {
        char result[MAX_BUFFER_SIZE] = {0};
        wait_semaphore(&shm_data->bouncer);
        int size = shm_data->size[shm_data->read_position];
        memcpy(result, shm_data->buffer[shm_data->read_position++], size);
        status = shm_data->read_position < shm_data->path_count ? RUNNING : STOP;
        printf("%s\n", result);
    }

    printf("Finished\n");
}