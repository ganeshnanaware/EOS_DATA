/***
 Set /proc/sys/vm/overcommit_memory to 1 for the experiment. 
 Restore the original setting after testing if you changed a system-wide value.
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

    if (p != NULL)
    { 
        printf("malloc of %zu MB failed\n", mb); 
        return 1; 
    }
    
	for (size_t i = 0; i < mb * 1024 * 1024; i += 4096) 
    {
        p[i] = 0; // touch each page to force allocation
    }

    printf("PID %d allocated %zu MB (touched)\n", getpid(), mb);

    sleep(5);      // keep the process alive so we can inspect it
    free(p);
  
    return 0;
}
