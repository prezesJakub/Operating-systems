#include <stdio.h>
#include "collatz/collatz.h"

#define MAX_ITER 100

void test_number(int num) {
    int steps[MAX_ITER];
    int count = test_collatz_convergence(num, MAX_ITER, steps);

    if (count > 0) {
        printf("Collatz sequence for %d: ", num);
        for(int i=0; i<count; i++) {
            printf("%d ", steps[i]);
        }
        printf("\n");
    } else {
        printf("Collatz sequence for %d did not converge within %d iterations.\n", num, MAX_ITER);
    }
}

int main() {
    test_number(6);
    test_number(15);
    test_number(27);
    return 0;
}