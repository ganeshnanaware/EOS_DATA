#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/wait.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "pthread.h"
#include "semaphore.h"

#define FIFO_PATH "/tmp/simple_fifo"
#define SIZE 3
#define TOTAL_ITEM 3

int buffer[SIZE];
int count = 0;
pthread_mutex_t mutex;
sem_t empty_slot;
sem_t full_slot;
int fifo_fd;

void* producer(void* arg);
void* consumer(void *arg);

int main()
{

    pid_t pid[3];

    mkfifo(FIFO_PATH, 0666);

    for(int i = 0; i<TOTAL_ITEM; i++)
    {
        pid[i] = fork();

        if(pid[i] < 0)
        {
            perror("child creation failed\n");
            exit(EXIT_FAILURE);
        }

        if(pid[i] == 0)
        {
            if(i == 0)
            {

                pthread_mutex_init(&mutex, NULL);
                sem_init(&empty_slot, 0 , SIZE);
                sem_init(&full_slot, 0,0);

                fifo_fd = open(FIFO_PATH, O_WRONLY);

                pthread_t t1, t2;
                int id1 = 1, id2 = 2;

                pthread_create(&t1, NULL, producer, &id1);
                pthread_create(&t2, NULL, consumer, &id2);
    
                //terminating threads

                pthread_join(t1, NULL);
                pthread_join(t2, NULL);

                close(fifo_fd);
                pthread_mutex_destroy(&mutex);
                sem_destroy(&empty_slot);
                sem_destroy(&full_slot);

                exit(0);
            }
            else{
                exit(0);
            }
        }
    }
    // parent process code //

    int fifo_read_fd = open(FIFO_PATH, O_RDONLY);
    int received; 

    printf("Reading item from child 0 through FIFO.\n");
    for(int i = 0; i < SIZE; i++)
    {
        read(fifo_read_fd, &received, sizeof(int));
        printf("Parent received item -> %d\n", received);
    }

    close(fifo_read_fd);

    // cleaning gracefully //

    for(int i = 0; i<3; i++)
    {
        wait(NULL);
    }

    unlink(FIFO_PATH);
    printf("parent all tasks are finished cleanly\n");

    return 0;
}

void* producer(void* arg)
{
    for(int i = 0; i<=TOTAL_ITEM; i++)
    {
        sem_wait(&empty_slot);       //wait if buffer is full
        pthread_mutex_lock(&mutex);  //lock buffer

        buffer[count++] = i;                       //save values in buffer
        printf("producer created item-> %d\n", i);

        pthread_mutex_unlock(&mutex);             //unlock buffer
        sem_post(&full_slot);                     //signal item is ready
    }
    return NULL;
}

void* consumer(void* arg)
{
    for(int i = 0; i<TOTAL_ITEM; i++)
    {
        sem_wait(&full_slot);
        pthread_mutex_lock(&mutex);

        int item = buffer[--count];

        printf("consumer consumed item :%d -> sending to parent via FIFO\n", item);

        write(fifo_fd, &item, sizeof(int));

        pthread_mutex_unlock(&mutex);             //unlock buffer
        sem_post(&empty_slot); 
    }

    return NULL;
}