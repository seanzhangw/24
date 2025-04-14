#include <stdio.h>
#include <stdbool.h>

// #include "permute.h"
#include "utils/permute.h"

double calc(int i, double num, double num2);
double calc2(int op, int op2, int op3, int arr[], int var);

bool has_solution(int a, int b, int c, int d)
{
    int numbers[4] = {a, b, c, d};
    double val;

    // Sort the array
    sortArray(4, numbers);

    // Iterate through all permutations and operator combinations
    do
    {
        for (int i = 0; i < 4; i++) // Operator 1
        {
            for (int j = 0; j < 4; j++) // Operator 2
            {
                for (int k = 0; k < 4; k++) // Operator 3
                {
                    for (int l = 0; l < 6; l++) // Parentheses arrangement
                    {
                        val = calc2(i, j, k, numbers, l);
                        if (24 - 0.0001 < val && val < 24 + 0.0001) // Check if close to 24
                        {
                            return true;
                        }
                    }
                }
            }
        }
    } while (findNext(4, numbers)); // Get the next permutation

    return false;
}

double calc(int op, double num, double num2)
{
    double returnVal;
    switch (op)
    {
    case 0: // add
        returnVal = num + num2;
        break;
    case 1: // subtract
        returnVal = num - num2;
        break;
    case 2: // multiply
        returnVal = num * num2;
        break;
    case 3: // divide
        if (num2 == 0)
        {
            num2 = 0.0000001; // Avoid division by zero
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
    case 0: // 123
        returnVal = calc(op, arr[0], arr[1]);
        returnVal = calc(op2, returnVal, arr[2]);
        returnVal = calc(op3, returnVal, arr[3]);
        break;
    case 1: // 132
    case 4: // 312
        returnVal = calc(op, arr[0], arr[1]);
        returnVal2 = calc(op3, arr[2], arr[3]);
        returnVal = calc(op2, returnVal, returnVal2);
        break;
    case 2: // 213
        returnVal2 = calc(op2, arr[1], arr[2]);
        returnVal = calc(op, arr[0], returnVal2);
        returnVal = calc(op3, returnVal, arr[3]);
        break;
    case 3: // 231
        returnVal2 = calc(op2, arr[1], arr[2]);
        returnVal = calc(op3, returnVal2, arr[3]);
        returnVal = calc(op, arr[0], returnVal);
        break;
    case 5: // 321
        returnVal2 = calc(op3, arr[2], arr[3]);
        returnVal = calc(op2, arr[1], returnVal2);
        returnVal = calc(op, arr[0], returnVal);
        break;
    }
    return returnVal;
}