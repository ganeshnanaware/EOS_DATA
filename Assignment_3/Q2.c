#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

int main() {
    pid_t pids[5];
    int i;

    for (i = 0; i < 5; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pids[i] == 0)
        {
    
            switch (i) {
                case 0:
          
                    execlp("./add.out", "add.out", "10", "20", NULL);

                    perror("execlp add failed");
                    exit(127);

                case 1:
                    
                    execlp("./zero.out", "zero.out", "50", "0", NULL);
                    perror("execlp zero failed");
                    exit(127);

                case 2:

                    execlp("./nonnumeric.out", "nonnumeric.out", "abc", "5", NULL);
                    perror("execlp nonnumeric failed");
                    exit(127);

                case 3:
                 
                    execlp("./while1.out", "while1.out", NULL);
                    perror("execlp while1 failed");
                    exit(127);

               		}
          	}
          }
              
    printf("Parent Process (PID: %d) successfully spawned 5 calculator child processes.\n", getpid());
    sleep(1); 

    printf("Parent sending SIGKILL to Child 3 (PID: %d - infinite loop calculator) to demonstrate abnormal termination.\n", pids[3]);
    kill(pids[3], SIGKILL);

    int status;
    pid_t terminated_pid;
    int children_left = 5;

    printf("\n--- Collecting and Interpreting Calculator Child Termination Statuses ---\n");
    
    while (children_left > 0) 
    {
        terminated_pid = waitpid(-1, &status, 0);
        if (terminated_pid < 0) {
            if (errno == ECHILD) break;
            perror("waitpid error");
            break;
        }


        children_left--;
        printf("\nCalculator Child PID %d terminated.\n", terminated_pid);

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            printf("  -> Normal termination.\n");
            printf("  -> Exit Status Code: %d\n", exit_status);
            
            if (exit_status == 0) {
                printf("  -> Result: SUCCESSFUL calculation/process completion.\n");
            } else {
                printf("  -> Result: UNSUCCESSFUL termination (non-zero exit code).\n");
            }
        } 
        else if (WIFSIGNALED(status)) {
            int sig_num = WTERMSIG(status);
            printf("  -> Abnormal termination.\n");
            printf("  -> Terminated by signal: %d (%s)\n", sig_num, strsignal(sig_num));
        }
    }

    printf("\nAll calculator processes have been reaped. Parent exiting.\n");
    
    return 0;
    
   }
    
 
   

