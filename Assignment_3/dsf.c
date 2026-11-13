#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define NUM_FILES 5

int main() {
    // Array of the 5 source files and their corresponding target object files
    char *sources[NUM_FILES] = {"add.c", "zero.c", "nonnumeric.c", "while1.c", "segfault.c"};
    char *objects[NUM_FILES] = {"add.o", "zero.o", "nonnumeric.o", "while1.o", "segfault.o"};
    
    pid_t pids[NUM_FILES];
    int i;

    printf("=== Q3.1 & 3.2: Spawning 5 Children for Parallel Compilation ===\n");

    // 1. Create five child processes from a common parent
    // 2. Compile five different source files into their respective object files
    for (i = 0; i < NUM_FILES; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pids[i] == 0) {
            // Child Process Code: Compile source to object file (gcc -c source.c -o source.o)
            // Note: Adding -lm for nonnumeric.c if it uses math library functions
            if (strcmp(sources[i], "nonnumeric.c") == 0) {
                execlp("gcc", "gcc", "-c", sources[i], "-o", objects[i], "-lm", NULL);
            } else {
                execlp("gcc", "gcc", "-c", sources[i], "-o", objects[i], NULL);
            }
            
            // If execlp fails
            perror("execlp compilation failed");
            exit(127);
        }
    }

    // 3. The parent must use waitpid() to collect the termination status of all children
    int status;
    pid_t terminated_pid;
    int children_left = NUM_FILES;
    int compilation_success = 1;

    printf("\n=== Q3.3: Collecting Compilation Statuses via waitpid() ===\n");

    while (children_left > 0) {
        terminated_pid = waitpid(-1, &status, 0);
        if (terminated_pid < 0) {
            if (errno == ECHILD) break;
            perror("waitpid error");
            break;
        }

        children_left--;

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == 0) {
                printf("[SUCCESS] Child PID %d compiled its source file successfully.\n", terminated_pid);
            } else {
                printf("[ERROR] Child PID %d compilation failed with exit code %d.\n", terminated_pid, exit_code);
                compilation_success = 0;
            }
        } else {
            printf("[ABNORMAL] Child PID %d terminated abnormally during compilation.\n", terminated_pid);
            compilation_success = 0;
        }
    }

    // 4. Based on exit status information, link the generated object files to create the final executable
    if (!compilation_success) {
        fprintf(stderr, "\n[FATAL] Compilation failed for one or more files. Aborting linking phase.\n");
        exit(EXIT_FAILURE);
    }

    printf("\n=== Q3.4: All compilations passed. Linking object files into 'final_calc' ===\n");
    
    pid_t link_pid = fork();
    if (link_pid < 0) {
        perror("fork for linking failed");
        exit(EXIT_FAILURE);
    }

    if (link_pid == 0) {
        // Child process executes the linker command
        // Note: Linking object files together. If main() is defined in one of them (e.g., add.c or a stub), linking succeeds.
        execlp("gcc", "gcc", objects[0], objects[1], objects[2], objects[3], objects[4], "-o", "final_calc", "-lm", NULL);
        perror("execlp linking failed");
        exit(127);
    }

    // Parent waits for the linker process to complete
    int link_status;
    waitpid(link_pid, &link_status, 0);

    if (WIFEXITED(link_status) && WEXITSTATUS(link_status) == 0) {
        printf("[SUCCESS] Linking complete! Executable 'final_calc' generated successfully.\n");
    } else {
        fprintf(stderr, "[ERROR] Linking failed (Ensure at least one source file defines 'int main()').\n");
        exit(EXIT_FAILURE);
    }

    // 5. If linking succeeds, load and execute the generated binary in a new process
    printf("\n=== Q3.5: Loading and Executing 'final_calc' in a New Process ===\n");
    
    pid_t exec_pid = fork();
    if (exec_pid < 0) {
        perror("fork for execution failed");
        exit(EXIT_FAILURE);
    }

    if (exec_pid == 0) {
        // Load and execute the newly generated binary
        execlp("./final_calc", "final_calc", NULL);
        perror("execlp execution failed");
        exit(127);
    }

    // Parent waits for the final application process to complete
    int final_status;
    waitpid(exec_pid, &final_status, 0);
    printf("\n=== Pipeline Complete. Parent Process Exiting. ===\n");

    return 0;
}