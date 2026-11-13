#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int global_var = 10;                 // data segment
int uninitialized_var;              // bss segment

int main() {
    int local_var = 5;               // stack
    int *heap_var = malloc(sizeof(int));   // heap

    printf("PID            : %d\n", getpid());
    printf("Code (main)    : %p\n", (void *)main);
    printf("Data (global)  : %p\n", (void *)&global_var);
    printf("Heap (malloc)  : %p\n", (void *)heap_var);
    printf("Stack (local)  : %p\n", (void *)&local_var);
    printf("libc (printf)  : %p\n", (void *)printf);

    printf("\nPress Enter to exit (check /proc/%d/maps meanwhile)...\n", getpid());
    getchar();
    free(heap_var);
    return 0;
}