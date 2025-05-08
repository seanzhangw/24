#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// #include "permute.h"
#include "reference_code/permute.h"
#include "array_collection_difficultylevel.h"

#define MAX_SIZE 100

int arrSol[MAX_SIZE][6];

double calc(int i, double num, double num2);
double calc2(int op, int op2, int op3, int arr[], int var);

// Give a permutation of n integers saved in the array x,
// find the next permutation in lexicographical order, and
// save it into the array x.
// Return 1 if next permutation is found. Otherwise, return 0.
int findNext(int n, int x[])
{
    int i;
    for (i = n - 1; i > 0; i--)
    {
        if (x[i - 1] < x[i])
        { // x[i-1] is first drop, to be increased
            for (int j = i; j < n; j++)
            {
                // find x[j] that is the smallest one great than x[i-1]
                if (j == n - 1 || x[j + 1] <= x[i - 1])
                {
                    // swap x[i-1] and x[j] (x[i-1] is incresed to x[j])
                    int temp = x[j];
                    x[j] = x[i - 1];
                    x[i - 1] = temp;
                    // reverse elements after x[i-1] from decending to ascending
                    for (int p = i, q = n - 1; p < q; p++, q--)
                    {
                        int temp = x[p];
                        x[p] = x[q];
                        x[q] = temp;
                    }
                    return 1; // found it
                }
            }
        }
    }
    // i==0, no drop, x in descending order, last permutation
    return 0; // no more
}

// Print out the array in one line
void printArray(int n, int a[])
{
    for (int i = 0; i < n; i++)
    {
        printf("%d ", a[i]);
    }
    printf("\n");
}

// Similar to strcmp
// Compare two integers, to be used in qsort to sort integers
int compare_ints(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

// Sort the array in ascending order
void sortArray(int n, int a[]) { qsort(a, 4, sizeof(int), compare_ints); }

int countSolutions(int a, int b, int c, int d)
{
    int numbers[4] = {a, b, c, d};
    double val;
    int count = 0;

    sortArray(4, numbers);

    do
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                for (int k = 0; k < 4; k++)
                {
                    for (int l = 0; l < 6; l++)
                    {
                        val = calc2(i, j, k, numbers, l);
                        if (24 - 0.0001 < val && val < 24 + 0.0001)
                        {
                            count++;
                        }
                    }
                }
            }
        }
    } while (findNext(4, numbers));

    return count;
}

bool has_solution(int a, int b, int c, int d)
{
    return countSolutions(a, b, c, d) > 0;
}

double calc(int op, double num, double num2)
{
    double returnVal;
    switch (op)
    {
    case 0:
        returnVal = num + num2;
        break;
    case 1:
        returnVal = num - num2;
        break;
    case 2:
        returnVal = num * num2;
        break;
    case 3:
        if (num2 == 0)
        {
            num2 = 0.0000001;
        }
        returnVal = num / num2;
        break;
    }
    return returnVal;
}

double calc2(int op, int op2, int op3, int arr[], int var)
{
    double returnVal;
    double returnVal2;
    switch (var)
    {
    case 0:
        returnVal = calc(op, arr[0], arr[1]);
        returnVal = calc(op2, returnVal, arr[2]);
        returnVal = calc(op3, returnVal, arr[3]);
        break;
    case 1:
    case 4:
        returnVal = calc(op, arr[0], arr[1]);
        returnVal2 = calc(op3, arr[2], arr[3]);
        returnVal = calc(op2, returnVal, returnVal2);
        break;
    case 2:
        returnVal2 = calc(op2, arr[1], arr[2]);
        returnVal = calc(op, arr[0], returnVal2);
        returnVal = calc(op3, returnVal, arr[3]);
        break;
    case 3:
        returnVal2 = calc(op2, arr[1], arr[2]);
        returnVal = calc(op3, returnVal2, arr[3]);
        returnVal = calc(op, arr[0], returnVal);
        break;
    case 5:
        returnVal2 = calc(op3, arr[2], arr[3]);
        returnVal = calc(op2, arr[1], returnVal2);
        returnVal = calc(op, arr[0], returnVal);
        break;
    }
    return returnVal;
}

int (*array_solutions(int size))[6]
{
    if (size > MAX_SIZE)
        return NULL;

    int count = 0;
    srand(time_us_64()); // Seed the random number generator with the current time

    while (count < size)
    {
        int a = rand() % 10 + 1;
        int b = rand() % 10 + 1;
        int c = rand() % 10 + 1;
        int d = rand() % 10 + 1;

        int num_solutions = countSolutions(a, b, c, d);

        if (num_solutions > 0)
        {
            arrSol[count][0] = a;
            arrSol[count][1] = b;
            arrSol[count][2] = c;
            arrSol[count][3] = d;

            if (num_solutions < 30)
                arrSol[count][4] = 2; // hard
            else if (num_solutions > 50)
                arrSol[count][4] = 0; // easy
            else
                arrSol[count][4] = 1; // medium

            arrSol[count][5] = num_solutions; // number of solutions
            count++;
        }
    }

    return arrSol;
}
