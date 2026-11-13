/* Attempt to access an invalid or unused virtual address. */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	
    int *ptr1 = (int *)malloc(sizeof(int));

    puts("Attempting to write to the unused address");
    
    *ptr1 = 42; 

    printf("Value at ptr1: %d\n", *ptr1);

    free(ptr1);
    getchar();

    int *ptr = NULL; // Pointer to an invalid address (NULL)

    puts("Attempting to access an invalid virtual address...");
	printf("Value: %d\n", *ptr);

    return 0;
}
