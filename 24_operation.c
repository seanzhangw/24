#include <stdio.h>

#include "permute.h"
/*
    Peter Zhang
    10/4/2019
    24 Game Solver
    This program takes in four integers and then uses functions and for loops to
    calculate all 64 combinations of all permutations, but returns the first
    solution found or no solution if there is none.
*/

double calc(int i, double num, double num2);
double calc2(int op, int op2, int op3, int arr[], int var);
void PrintStuff(int op, int op2, int op3, int arr[], int var);

int main() {
    int numbers[4];
    double val;
    printf("Enter 4 numbers: \n");
    scanf("%d%d%d%d", &numbers[0], &numbers[1], &numbers[2], &numbers[3]);
    // printf("%lf", calc2(add, add, div, numbers));
    sortArray(4, numbers);
    do {
        // printArray(4,numbers);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    for (int l = 0; l < 6; l++) {
                        val = calc2(i, j, k, numbers, l);
                        if (24 - 0.0001 < val && val < 24 + 0.0001) {
                            PrintStuff(i, j, k, numbers, l);
                            return 1;
                        }
                    }
                }
            }
        }
    } while (findNext(4, numbers));
    printf("No solution\n");
    return 0;
}

double calc(int op, double num, double num2) {
    // i = operation,num and num2 are the numbers to operate
    double returnVal;
    switch (op) {
        case 0:  // add
            returnVal = (double)num + num2;
            break;
        case 1:  // subtract
            returnVal = (double)num - num2;
            break;
        case 2:  // multiply
            returnVal = (double)num * num2;
            break;
        case 3:  // divide
            if (num2 == 0) {
                num2 = 0.0000001;
            }
            returnVal = (double)num / num2;
            // printf("%g %g\n", num, num2);
            break;
    }
    return returnVal;
}

double calc2(int op, int op2, int op3, int arr[], int var) {
    double returnVal;
    double returnVal2;
    switch (var) {
        case 0:  // 123
            returnVal = calc(op, arr[0], arr[1]);
            returnVal = calc(op2, returnVal, arr[2]);
            returnVal = calc(op3, returnVal, arr[3]);
            break;
        case 1:  // 132
        case 4:  // 312
            returnVal = calc(op, (double)arr[0], (double)arr[1]);
            returnVal2 = calc(op3, (double)arr[2], (double)arr[3]);
            returnVal = calc(op2, returnVal, returnVal2);
            break;
        case 2:  // 213
            returnVal2 = calc(op2, arr[1], arr[2]);
            returnVal = calc(op, arr[0], returnVal2);
            returnVal = calc(op3, returnVal, arr[3]);
            break;
        case 3:  // 231
            returnVal2 = calc(op2, arr[1], arr[2]);
            returnVal = calc(op3, returnVal2, arr[3]);
            returnVal = calc(op, arr[0], returnVal);
            break;
        case 5:  // 321
            returnVal2 = calc(op3, arr[2], arr[3]);
            returnVal = calc(op2, arr[1], returnVal2);
            returnVal = calc(op, arr[0], returnVal);
            break;
    }
    return returnVal;
}
void PrintStuff(int op, int op2, int op3, int arr[], int var) {
    char operators[4] = {'+', '-', '*', '/'};
    switch (var) {
        case 0:  // 123
            printf("((%d %c %d) %c %d) %c %d", arr[0], operators[op], arr[1],
                   operators[op2], arr[2], operators[op3], arr[3]);
            break;
        case 1:  // 132
        case 4:  // 312
            printf("(%d %c %d) %c (%d %c %d)", arr[0], operators[op], arr[1],
                   operators[op2], arr[2], operators[op3], arr[3]);
            break;
        case 2:  // 213
            printf("(%d %c (%d %c %d)) %c %d", arr[0], operators[op], arr[1],
                   operators[op2], arr[2], operators[op3], arr[3]);
            break;
        case 3:  // 231
            printf("%d %c ((%d %c %d) %c %d)", arr[0], operators[op], arr[1],
                   operators[op2], arr[2], operators[op3], arr[3]);
            break;
        case 5:  // 321
            printf("%d %c (%d %c (%d %c %d))", arr[0], operators[op], arr[1],
                   operators[op2], arr[2], operators[op3], arr[3]);
            break;
    }
    printf(" = 24\n");
}