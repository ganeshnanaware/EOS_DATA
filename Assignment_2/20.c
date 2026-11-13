//20.Attempt to access a selected address in the read-only data segment

#include <stdio.h>
#include <unistd.h>

 int global_var = 10;                   // data segment
 const int read_only_var = 20;          // read-only data segment

int main(void) 
{
    int *ptr = &global_var;         

    printf("Address of pointer : %p\n", (void *)ptr);
    printf("value before modification : %d\n", *ptr);

    *ptr = 50; // Modify the value of the global variable
    
    printf("value after modification : %d\n", *ptr);

    getchar();

    // Attempt to modify the read-only variable
    printf("\nAttempting to modify read-only variable...\n");
    *((int *)&read_only_var) = 30; 

    printf("New value of read-only variable: %d\n", read_only_var);
    printf("\nPress Enter to exit (check /proc/%d/maps meanwhile)...\n", getpid());

    getchar();
    return 0;
}