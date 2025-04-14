#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Include the necessary header file
#include "utils/permute.h"

/*
    This program reads numbers from a file (collection_with_solution.txt),
    calculates the solution for each line, and appends the calculation method
    to the output. 
    v1: only one solution is printed
    v2: all solutions are printed
    problem?: negative numbers will appear in the operation process
*/

double calc(int i, double num, double num2);
double calc2(int op, int op2, int op3, int arr[], int var);
void PrintStuffToFile(FILE *outputFile, int op, int op2, int op3, int arr[], int var);

int main()
{
    FILE *inputFile = fopen("collection_with_solution.txt", "r");
    if (inputFile == NULL)
    {
        perror("Failed to open input file");
        return EXIT_FAILURE;
    }

    FILE *outputFile = fopen("collection_with_solution_with_methods.txt", "w");
    if (outputFile == NULL)
    {
        perror("Failed to open output file");
        fclose(inputFile);
        return EXIT_FAILURE;
    }

    int numbers[4];
    double val;

    // Read each line from the input file
    while (fscanf(inputFile, "%d %d %d %d", &numbers[0], &numbers[1], &numbers[2], &numbers[3]) == 4)
    {
        fprintf(outputFile, "%d %d %d %d: ", numbers[0], numbers[1], numbers[2], numbers[3]);

        // Sort the array
        sortArray(4, numbers);

        bool solutionFound = false;

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
                            if (24 - 0.0001 < val && val < 24 + 0.0001)
                            {
                                PrintStuffToFile(outputFile, i, j, k, numbers, l);
                                solutionFound = true;
                                break;
                            }
                        }
                        if (solutionFound)
                            break;
                    }
                    if (solutionFound)
                        break;
                }
                if (solutionFound)
                    break;
            }
        } while (!solutionFound && findNext(4, numbers)); // Get the next permutation

        if (!solutionFound)
        {
            fprintf(outputFile, "No solution\n");
        }
    }

    fclose(inputFile);
    fclose(outputFile);

    printf("Results have been written to collection_with_solution_with_methods.txt\n");
    return 0;
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

void PrintStuffToFile(FILE *outputFile, int op, int op2, int op3, int arr[], int var)
{
    char operators[4] = {'+', '-', '*', '/'};
    switch (var)
    {
    case 0: // 123
        fprintf(outputFile, "((%d %c %d) %c %d) %c %d\n", arr[0], operators[op], arr[1],
                operators[op2], arr[2], operators[op3], arr[3]);
        break;
    case 1: // 132
    case 4: // 312
        fprintf(outputFile, "(%d %c %d) %c (%d %c %d)\n", arr[0], operators[op], arr[1],
                operators[op2], arr[2], operators[op3], arr[3]);
        break;
    case 2: // 213
        fprintf(outputFile, "(%d %c (%d %c %d)) %c %d\n", arr[0], operators[op], arr[1],
                operators[op2], arr[2], operators[op3], arr[3]);
        break;
    case 3: // 231
        fprintf(outputFile, "%d %c ((%d %c %d) %c %d)\n", arr[0], operators[op], arr[1],
                operators[op2], arr[2], operators[op3], arr[3]);
        break;
    case 5: // 321
        fprintf(outputFile, "%d %c (%d %c (%d %c %d))\n", arr[0], operators[op], arr[1],
                operators[op2], arr[2], operators[op3], arr[3]);
        break;
    }
}