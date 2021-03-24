#include "include/slave.h"

int main(
    int argc, 
    char **argv) 
{
    printf("[SLAVE] I was summoned!! my arg count is %d\n", argc);

    for(int i = 0; i < argc; i++)
    {
        printf("Arg[%d]: %s\n", i, argv[i]);
    }
    
    return 0;
}