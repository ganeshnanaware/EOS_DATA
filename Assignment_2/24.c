#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    { 
        printf("usage: %s <MB>\n", argv[0]); 
        return 1; 
    }
    size_t mb = atol(argv[1]);

    char *p = malloc(mb * 1024 * 1024);

    if (p == NULL)
     { 
        printf("malloc of %zu MB failed\n", mb); 
        return 1; 
    }
    printf("PID %d allocated %zu MB (not touched)\n", getpid(), mb);

    sleep(10);      // keep the process alive so we can inspect it
    free(p);
    
    return 0;
}
