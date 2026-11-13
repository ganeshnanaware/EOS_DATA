#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define NUM_CHILDREN 4

int main()
{
    
    char *sources[NUM_CHILDREN] = {"main_prog.c", "add.c", "zero.c", "NonNumeric.c"};
    char *objects[NUM_CHILDREN] = {"main_prog.o", "add.o", "zero.o", "NonNumeric.o"};

    pid_t pids[NUM_CHILDREN];
    int i;

    printf("=== Q3.1 & 3.2: Spawning 4 child processes for parallel compilation ===\n");

    for(i = 0; i < NUM_CHILDREN; i++)
    {
        pids[i] = fork();

        if(pids[i] < 0)
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if(pids[i] == 0)
        {
           
            if(strcmp(sources[i], "NonNumeric.c") == 0)
            {
                execlp("gcc", "gcc", "-c", sources[i], "-o", objects[i], "-lm", NULL);
            }
            else
            {
                execlp("gcc", "gcc", "-c", sources[i], "-o", objects[i], NULL);
            }

            perror("execlp compilation failed");
            exit(EXIT_FAILURE);
        }
    }

    int status;
    pid_t terminated_pid;
    int children_left = NUM_CHILDREN;
    int compilation_success = 1;

    printf("=== Q3.3: Collecting compilation statuses via waitpid() ===\n");

    while(children_left > 0)
    {
        terminated_pid = waitpid(-1, &status, 0);
        if(terminated_pid < 0)
        {
            if(errno == ECHILD) break;
            perror("waitpid error");
            break;
        }

        children_left--;

        if(WIFEXITED(status))
        {
            int exit_code = WEXITSTATUS(status);
            if(exit_code == 0)
            {
                printf("[SUCCESS] Child PID %d compiled its source file successfully.\n", terminated_pid);
            }
            else
            {
                printf("[UNSUCCESSFUL] Child PID %d compilation failed with exit code %d.\n", terminated_pid, exit_code);
                compilation_success = 0;
            }
        }
        else
        {
            printf("[ABNORMAL] Child PID %d terminated abnormally during compilation.\n", terminated_pid);
            compilation_success = 0;
        }
    }

    if(!compilation_success)
    {
        fprintf(stderr, "\n[FATAL] Compilation failed for one or more files. Aborting linking phase.\n");
        exit(EXIT_FAILURE);
    }

    printf("\n=== Q3.4: All compilations passed. Linking object files into 'final_calc'... ===\n");

    pid_t link_pid = fork();
    if(link_pid < 0)
    {
        perror("fork for linking failed");
        exit(EXIT_FAILURE);
    }

    if(link_pid == 0)
    {
        
        execlp("gcc", "gcc", "main_prog.o", "add.o", "zero.o", "NonNumeric.o", "-o", "final_calc", "-lm", NULL);
        perror("execlp linking failed");
        exit(EXIT_FAILURE);
    }

    int link_status;
    waitpid(link_pid, &link_status, 0);

    if(WIFEXITED(link_status) && WEXITSTATUS(link_status) == 0)
    {
        printf("[SUCCESS] Linking complete! 'final_calc' generated successfully.\n");
    }
    else
    {
        fprintf(stderr, "[ERROR] Linking failed.\n");
        exit(EXIT_FAILURE);
    }

    printf("\n=== Q3.5: Loading and Executing 'final_calc' in a new process ===\n");

    pid_t exec_pid = fork();
    if(exec_pid < 0)
    {
        perror("fork for execution failed");
        exit(EXIT_FAILURE);
    }

    if(exec_pid == 0)
    {
        execlp("./final_calc", "final_calc", NULL);
        perror("execlp execution failed");
        exit(127);
    }

    int final_status;
    waitpid(exec_pid, &final_status, 0);
    printf("=== Pipeline Completed Successfully. Parent Exiting. ===\n");

    return 0;
}