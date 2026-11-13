/***
Launch multiple instances of ex2 with different arguments, 
for example: ./ex2 10 &, ./ex2 100 &, ./ex2 1000 &, ./ex2 2000 &.
***/

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
    for (size_t i = 0; i < mb * 1024 * 1024; i += 4096) 
    {
        p[i] = 0; // touch each page to force allocation
    }

    printf("PID %d allocated %zu MB (not touched)\n", getpid(), mb);

    while(1);      // keep the process alive so we can inspect it
    free(p);
  
    return 0;
}
