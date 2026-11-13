#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

#define MSG_KEY 0x5678


struct message {
    long mtype;
    int data;
};


int single_slot = 0;
int has_data = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


void* producer_func(void *arg) {
    int msqid = *(int*)arg;
    struct message msg;
    msg.mtype = 1;

    for (int i = 1; i <= 10; i++) {
        msg.data = 2 * i; 
        msgsnd(msqid, &msg, sizeof(msg.data), 0);
        printf("[Producer] Sent even number: %d\n", msg.data);
        usleep(100000);
    }
    return NULL;
}


void* consumer_func(void *arg) {
    int msqid = *(int*)arg;
    struct message msg;

    for (int i = 0; i < 10; i++) {
        msgrcv(msqid, &msg, sizeof(msg.data), 1, 0);

        pthread_mutex_lock(&mutex);
        while (has_data == 1) { 
            pthread_cond_wait(&cond, &mutex);
        }

        single_slot = msg.data;
        has_data = 1;
        printf("[Consumer] Received %d -> placed in buffer slot\n", single_slot);

        pthread_cond_signal(&cond); // Wake up processor thread
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}


void* processor_func(void *arg) {
    FILE *file = fopen("output.txt", "w");
    if (!file) {
        perror("fopen failed");
        pthread_exit(NULL);
    }

    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        while (has_data == 0) { // Wait if buffer is empty
            pthread_cond_wait(&cond, &mutex);
        }

        int val = single_slot;
        has_data = 0;
        
        int square = val * val;
        printf("[Processor] Processed: %d, Square: %d -> Writing to file\n", val, square);
        fprintf(file, "Number: %d, Square: %d\n", val, square);
        fflush(file);

        pthread_cond_signal(&cond); // Wake up consumer thread
        pthread_mutex_unlock(&mutex);
    }

    fclose(file);
    return NULL;
}

int main(void) {
    // Step A: Create System V Message Queue
    int msqid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msqid < 0) {
        perror("msgget failed");
        exit(1);
    }

    pid_t pid1, pid2;

    // Step B: Fork Child 1 (Producer Process)
    pid1 = fork();
    if (pid1 < 0) {
        perror("fork 1 failed");
        exit(1);
    }
    if (pid1 == 0) {
        pthread_t tid;
        pthread_create(&tid, NULL, producer_func, &msqid);
        pthread_join(tid, NULL);
        exit(0);
    }

    // Step C: Fork Child 2 (Consumer + Processor Process)
    pid2 = fork();
    if (pid2 < 0) {
        perror("fork 2 failed");
        exit(1);
    }
    if (pid2 == 0) {
        pthread_t t_consumer, t_processor;

        pthread_create(&t_consumer, NULL, consumer_func, &msqid);
        pthread_create(&t_processor, NULL, processor_func, NULL);

        pthread_join(t_consumer, NULL);
        pthread_join(t_processor, NULL);
        exit(0);
    }

    // Step D: Parent waits for both children, then removes message queue
    wait(NULL);
    wait(NULL);

    msgctl(msqid, IPC_RMID, NULL);
    printf("\n[Parent] All items processed. Message queue deleted. Exiting.\n");

    return 0;
}