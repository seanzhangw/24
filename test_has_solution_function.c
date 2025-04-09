#include <stdio.h>
#include <stdbool.h>
/*
gcc test_has_solution_function.c solve_24_Game_function.c permute.c -o test_has_solution
*/

// 声明 has_solution 函数
bool has_solution(int a, int b, int c, int d);

int main()
{
    if (has_solution(1, 1, 1, 1))
    // if (has_solution(1, 2, 3, 4))
    // if (has_solution(1, 1, 1, 1))
    // if (has_solution(1, 2, 3, 5))
    // if (has_solution(1, 2, 3, 6))
    // if (has_solution(1, 2, 3, 7))
    {
        printf("Solution exists!\n");
    }
    else
    {
        printf("No solution.\n");
    }
    return 0;
}