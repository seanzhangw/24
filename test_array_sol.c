#include <stdio.h>
#include "array_collection_difficultylevel.h"

int main()
{
    int size = 10;
    int (*results)[5] = array_solutions(size);

    if (results == NULL)
    {
        printf("Error: too many requested samples or allocation failed.\n");
        return 1;
    }

    printf("Generated %d solvable sets with difficulty levels:\n", size);
    for (int i = 0; i < size; i++)
    {
        int a = results[i][0];
        int b = results[i][1];
        int c = results[i][2];
        int d = results[i][3];
        int level = results[i][4]; // 0: easy, 1: medium, 2: hard

        const char *label = level == 0 ? "easy" : (level == 1 ? "medium" : "hard");
        printf("[%d %d %d %d] -> %s\n", a, b, c, d, label);
    }

    return 0;
}
