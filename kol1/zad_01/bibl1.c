#include <stdlib.h>
#include <stdio.h>
#include "bibl1.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

/*napisz biblioteke ladowana dynamicznie przez program zawierajaca funkcje:

1) zliczajaca sume n elementow tablicy tab:
int sumuj(int *tab, int n);

2) wyznaczajaca średnią n elementow tablicy tab
double srednia(int *tab, int n);

3) wyznaczajaca mediane n elementow tablicy tab
double mediana(int *tab, int n);

*/

int sumuj(int *tab, int n) {
    int sum = 0;
    for (int i=0; i<n; i++) {
        sum += tab[i];
    }
    return sum;
}

double srednia(int *tab, int n) {
    if (n == 0) return 0.0;
    return (double)sumuj(tab, n) / n;
}

int compare_ints(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

double mediana(int *tab, int n) {
    int *tmp = malloc(sizeof(int) * n);
    memcpy(tmp, tab, sizeof(int) * n);
    qsort(tmp, n, sizeof(int), compare_ints);

    double result;
    if (n%2 == 1) {
        result = tmp[n/2];
    } else {
        result = (tmp[n/2 - 1] + tmp[n/2]) / 2.0;
    }

    free(tmp);
    return result;
}