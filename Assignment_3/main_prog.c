#include <stdio.h>
#include "operations.h"

int main() {
    printf("--- Executing Functions from Linked Modules ---\n");
    perform_addition();
    perform_division();
    perform_sqrt_test();
    printf("--- Main Program Execution Complete ---\n");
    return 0;
}