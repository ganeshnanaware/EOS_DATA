#include <stdio.h>
#include "operations.h"

void perform_division() {
    int numerator = 50, denominator = 0;
    if (denominator == 0) {
        printf("[zero.c] Error: Division by zero avoided/handled.\n");
    } else {
        printf("[zero.c] Result: %d\n", numerator / denominator);
    }
}