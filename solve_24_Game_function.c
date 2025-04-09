#include <stdio.h>
#include <stdbool.h>

#include "permute.h"
/*
    Peter Zhang
    10/4/2019
    24 Game Solver
    This function takes in four integers and then uses functions and for loops to
    calculate all 64 combinations of all permutations, but returns true if a solution
    is found or false if there is none.
*/

double calc(int i, double num, double num2);
double calc2(int op, int op2, int op3, int arr[], int var);

bool has_solution(int a, int b, int c, int d)
{
    int numbers[4] = {a, b, c, d};
    double val;

    // 对数组进行排序
    sortArray(4, numbers);

    // 遍历所有排列和操作符组合
    do
    {
        for (int i = 0; i < 4; i++) // 操作符1
        {
            for (int j = 0; j < 4; j++) // 操作符2
            {
                for (int k = 0; k < 4; k++) // 操作符3
                {
                    for (int l = 0; l < 6; l++) // 括号排列
                    {
                        val = calc2(i, j, k, numbers, l);
                        if (24 - 0.0001 < val && val < 24 + 0.0001) // 检查是否接近24
                        {
                            return true; // 找到解，返回 true
                        }
                    }
                }
            }
        }
    } while (findNext(4, numbers)); // 获取下一个排列

    return false; // 无解，返回 false
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
            num2 = 0.0000001; // 避免除以零
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