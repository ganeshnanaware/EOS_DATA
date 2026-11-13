#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/mman.h"
#include "fcntl.h"
#include "sys/wait.h"
#include "string.h"

#define SHM_NAME "/free_writer"

int main()
{
    long page_size = sysconf(_SC_PAGE_SIZE);
    size_t total_size = (3*page_size);

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

    char *ptr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(ptr == MAP_FAILED)
    {
        perror("mmap failed\n");
        exit(0);
    }

    strcpy(ptr, "This is from writer : this is page 1\n");
    strcpy(ptr + page_size, "this is from writer : this is page 2\n");
    strcpy(ptr + (2 * page_size), " this is from writer : this is page 3\n");

    printf("data return across 3 pages successfully..\n");
    printf("Waiting for reader to read data so added little sleep\n");
    sleep(20);

    //cleanup

    munmap(ptr, total_size);
    close(fd);
    shm_unlink(SHM_NAME);

    printf("Writer has finished writing and exited..\n");

    return 0;

}