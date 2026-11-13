#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define NUM_CHILDREN 5

int main(void) {
    pid_t pids[NUM_CHILDREN];
    int i;

    printf("[Parent] PID: %d. Spawning %d children...\n", getpid(), NUM_CHILDREN);

    // Creating five child processes
    for (i = 0; i < NUM_CHILDREN; i++) {
        pids[i] = fork();
        
        if (pids[i] < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } 
        else if (pids[i] == 0) {
            
            pid_t cp = getpid();
            printf("Child %d PID: %d started.\n", i + 1, cp);

            if (i == 0) {
               
                exit(0);
            } 
            else if (i == 1) {
               
                exit(42);
            } 
            else {
                
                while (1) {
                    sleep(1);
                }
            }
        }
    }


    printf("Parent Finished spawning. Doing parent work...\n");
    sleep(3); 
    printf("Parent Starting to collect terminated children via waitpid()...\n\n");

    
    int status;
    pid_t terminated_pid;
    int collected = 0;

    while (collected < NUM_CHILDREN) {
    
        terminated_pid = waitpid(-1, &status, 0);
        
        if (terminated_pid > 0) {
            collected++;
            printf("Parent Reaped child PID: %d\n", terminated_pid);

         
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                printf("  -> Normal termination. Exit status: %d", exit_code);
                if (exit_code == 0) {
                    printf(" Success\n");
                } else {
                    printf(" Unsuccessful\n");
                }
            } 
            else if (WIFSIGNALED(status)) {
                int sig = WTERMSIG(status);
                printf("  -> Abnormal termination. Killed by signal: %d (%s)\n", 
                       sig, strsignal(sig));
            }
        } else {
            break;
        }
    }

    printf("Parent All children collected. Exiting.\n");
    return 0;
}
