#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/mman.h"
#include "fcntl.h"
#include "sys/errno.h"
#include "sys/wait.h"
#include "string.h"

#define SHM_NAME "/free_writer"

int main()
{
    long page_size = sysconf(_SC_PAGE_SIZE);
    size_t total_size = (3 * page_size);

    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if(fd == -1)
    {
        perror("shm_open failed..\n");
        exit(1);
    }

   char *ptr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

   if(ptr == MAP_FAILED)
   {
    perror("mapping failed\n");
    exit(0);
   }

   printf("Reader succefully attached to shared memory..\n");

   printf("Reader 1st -> %s\n", ptr);
   printf("reader 2nd -> %s\n", ptr + page_size);
   printf("reader 3rd -> %s\n", ptr + (2*page_size));

   munmap(ptr, total_size);
   close(fd);

   return 0;


}