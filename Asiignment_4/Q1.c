#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/mman.h"
#include "fcntl.h"
#include "sys/wait.h"
#include "string.h"


#define SHM_NAME "/my_simple_shm"

int main()
{
    //get system exact page size 
    long page_size = sysconf(_SC_PAGE_SIZE);
    size_t total_size = 3 * page_size;

    //create a shared memory object in the kernel

    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

    if(fd == -1)
    {
        perror("shm_open failed\n");
        exit(1);
    }

    if(ftruncate(fd, total_size) == -1)
    {
        perror("size allocation failed\n");
        exit(1);
    }

    char *ptr = mmap(NULL,total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(ptr == NULL)
    {
        perror("mmap failed..\n");
        exit(1);
    }

    strcpy(ptr, "hello from parent this is page 1.\n");
    strcpy(ptr+page_size, "hello from parent this is page 2\n");
    strcpy(ptr+(2*page_size), "hello from parent this is page 3\n");

    printf("Parent successfully wrote data into all 3 pages.\n");

    pid_t pid = fork();
    if(pid < 0)
    {
        perror("child creation failed.\n");
        exit(EXIT_FAILURE);
    }

    if(pid == 0)
    {
        printf("child reading data from shared memory..\n");
        printf("child reading first page -> %s\n", ptr);
        printf("Child reading second page -> %s\n", ptr + page_size);
        printf("child reading third page -> %s\n", ptr + (2 * page_size));
        exit(0);
    }

    wait(NULL);

    //cleanup resources

    munmap(ptr, total_size);
    close(fd);
    shm_unlink(SHM_NAME);

    printf("Parent cleaned up memory and exited\n");

    return 0;

}