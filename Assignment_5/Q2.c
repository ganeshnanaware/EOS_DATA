#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <semaphore.h>

#define SHM_KEY 0x1234
#define SIZE 10

typedef struct {
    int buffer[SIZE];
    int count;
    
    struct {
        int produced;
        int consumed;
    } status;

    pthread_mutex_t mutex;
    sem_t empty;
    sem_t full;
    pthread_rwlock_t rwlock;
} shared_mem;

void* producer(void *arg) {
    shared_mem *shm = (shared_mem *)arg;

    for (int i = 0; i < SIZE; i++) {
        sem_wait(&shm->empty);
        pthread_mutex_lock(&shm->mutex);

        shm->buffer[shm->count++] = i;
        printf("Produced: %d\n", i);

        pthread_rwlock_wrlock(&shm->rwlock);
        shm->status.produced++;
        pthread_rwlock_unlock(&shm->rwlock);

        pthread_mutex_unlock(&shm->mutex);
        sem_post(&shm->full);
        usleep(100000);
    }
    return NULL;
}

void* consumer(void *arg) {
    shared_mem *shm = (shared_mem *)arg;
    FILE *f = fopen("output.txt", "w");
    if (!f) {
        perror("fopen failed");
        pthread_exit(NULL);
    }

    for (int i = 0; i < SIZE; i++) {
        sem_wait(&shm->full);
        pthread_mutex_lock(&shm->mutex);

        int val = shm->buffer[--shm->count];
        printf("Consumed: %d\n", val);
        fprintf(f, "%d\n", val);
        fflush(f);

        pthread_rwlock_wrlock(&shm->rwlock);
        shm->status.consumed++;
        pthread_rwlock_unlock(&shm->rwlock);

        pthread_mutex_unlock(&shm->mutex);
        sem_post(&shm->empty);
        usleep(150000);
    }

    fclose(f);
    return NULL;
}

int main(void) {
    // 1. Remove any old shared memory segment with this key if it exists
    int old_id = shmget(SHM_KEY, sizeof(shared_mem), 0666);
    if (old_id >= 0) {
        shmctl(old_id, IPC_RMID, NULL);
    }

    // 2. Create and attach shared memory in parent
    int shmid = shmget(SHM_KEY, sizeof(shared_mem), IPC_CREAT | IPC_EXCL | 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    shared_mem *shm = (shared_mem *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    // 3. Initialize variables & sync tools
    shm->count = 0;
    shm->status.produced = 0;
    shm->status.consumed = 0;

    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shm->mutex, &mattr);
    pthread_mutexattr_destroy(&mattr);

    sem_init(&shm->empty, 1, SIZE);
    sem_init(&shm->full, 1, 0);

    pthread_rwlockattr_t rattr;
    pthread_rwlockattr_init(&rattr);
    pthread_rwlockattr_setpshared(&rattr, PTHREAD_PROCESS_SHARED);
    pthread_rwlock_init(&shm->rwlock, &rattr);
    pthread_rwlockattr_destroy(&rattr);

    shmdt(shm); // Detach so children attach cleanly

    // 4. Fork two children
    for (int i = 0; i < 2; i++) {
        if (fork() == 0) {
            shared_mem *c_shm = (shared_mem *)shmat(shmid, NULL, 0);
            if (c_shm == (void *)-1) {
                perror("child shmat failed");
                exit(1);
            }

            pthread_t tid;
            if (i == 0) pthread_create(&tid, NULL, producer, c_shm);
            else        pthread_create(&tid, NULL, consumer, c_shm);

            pthread_join(tid, NULL);
            shmdt(c_shm);
            exit(0);
        }
    }

    // 5. Parent waits, re-attaches to read status, and cleans up
    wait(NULL);
    wait(NULL);

    shm = (shared_mem *)shmat(shmid, NULL, 0);
    if (shm != (void *)-1) {
        printf("\nFinal -> Produced: %d, Consumed: %d\n", 
               shm->status.produced, shm->status.consumed);

        pthread_mutex_destroy(&shm->mutex);
        sem_destroy(&shm->empty);
        sem_destroy(&shm->full);
        pthread_rwlock_destroy(&shm->rwlock);
        shmdt(shm);
    }

    shmctl(shmid, IPC_RMID, NULL); // Remove shared memory
    return 0;
}