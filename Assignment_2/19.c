//20.Attempt to access a selected address in the read-only data segment

#include <stdio.h>
#include <unistd.h>

int main(void) 
{
    int *ptr = (int *)main; // Pointer to the main function (code segment) 

    printf("Address of pointer : %p\n", (void *)ptr);
    printf("value before modification : %d\n", *ptr);

    printf("\nAttempting to modify code segment...\n");
   // getchar();
    *ptr = 50; // Modify the value of the global variable
    
    printf("value after modification : %d\n", *ptr);

    return 0;
}