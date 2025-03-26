#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define MAX_ITER 100

int main() {
    void *handle = dlopen("./collatz/libcollatz.so", RTLD_LAZY);
    if(!handle) {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        return 1;
    }

    int (*test_collatz_convergence)(int, int, int *) = dlsym(handle, "test_collatz_convergence");
    if (!test_collatz_convergence) {
        fprintf(stderr, "Error finding function: %s\n", dlerror());
        return 1;
    }

    int steps[MAX_ITER];
    int count = test_collatz_convergence(26, MAX_ITER, steps);

    if(count > 0) {
        printf("Collatz sequence for 26: ");
        for (int i=0; i<count; i++) {
            printf("%d ", steps[i]);
        }
        printf("\n");
    } else {
        printf("Sequence did not converge within %d iterations.\n", MAX_ITER);
    }
    dlclose(handle);
    return 0;
}