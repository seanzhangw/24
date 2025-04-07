#include <stdio.h>
#include <stdlib.h>

// Give a permutation of n integers saved in the array x,
// find the next permutation in lexicographical order, and
// save it into the array x.
// Return 1 if next permutation is found. Otherwise, return 0.
int findNext(int n, int x[]) {
    int i;
    for (i = n - 1; i > 0; i--) {
        if (x[i - 1] < x[i]) {  // x[i-1] is first drop, to be increased
            for (int j = i; j < n; j++) {
                // find x[j] that is the smallest one great than x[i-1]
                if (j == n - 1 || x[j + 1] <= x[i - 1]) {
                    // swap x[i-1] and x[j] (x[i-1] is incresed to x[j])
                    int temp = x[j];
                    x[j] = x[i - 1];
                    x[i - 1] = temp;
                    // reverse elements after x[i-1] from decending to ascending
                    for (int p = i, q = n - 1; p < q; p++, q--) {
                        int temp = x[p];
                        x[p] = x[q];
                        x[q] = temp;
                    }
                    return 1;  // found it
                }
            }
        }
    }
    // i==0, no drop, x in descending order, last permutation
    return 0;  // no more
}

// Print out the array in one line
void printArray(int n, int a[]) {
    for (int i = 0; i < n; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}

// Similar to strcmp
// Compare two integers, to be used in qsort to sort integers
int compare_ints(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

// Sort the array in ascending order
void sortArray(int n, int a[]) { qsort(a, 4, sizeof(int), compare_ints); }