#include "include/slave.h"


int main(
    int argc, 
    char **argv) 
{
    char payload_size = 0;
    char *payload = NULL;

    while( read(STDIN, &payload_size, 1) == 1 )
    {
        payload = malloc(payload_size);
        if( read(STDIN, payload, payload_size) == payload_size )
        {
            write(STDOUT, &payload_size, 1);
            write(STDOUT, payload, payload_size);
        }
    }
    free(payload);
}