#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

/*
gcc generate_collection_with_solution.c solve_24_Game_function.c permute.c -o generate_collection_with_solution
*/

// 声明外部函数，假设 solve_24_Game_function.c 提供了一个函数 `has_solution`，用于判断是否有解
extern bool has_solution(int a, int b, int c, int d);

void generate_and_check()
{
    FILE *file = fopen("collection_with_solution.txt", "w");
    if (file == NULL)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL)); // 初始化随机数种子

    for (int i = 0; i < 1000; i++)
    {                            // 生成 1000 组随机数字
        int a = rand() % 13 + 1; // 生成 1~13 的随机数
        int b = rand() % 13 + 1;
        int c = rand() % 13 + 1;
        int d = rand() % 13 + 1;

        if (has_solution(a, b, c, d))
        {                                               // 调用 24_operation.c 的函数判断是否有解
            fprintf(file, "%d %d %d %d\n", a, b, c, d); // 将有解的组合写入文件
        }
    }

    fclose(file);
    printf("Collections have been written to collection_with_solution.txt\n");
}

int main()
{
    generate_and_check();
    return 0;
}