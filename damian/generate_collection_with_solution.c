#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

/*
gcc generate_collection_with_solution.c solve_24_Game_function.c permute.c -o generate_collection_with_solution
*/

// has_solution() is defined in the file "solve_24_Game_function.c"
extern bool has_solution(int a, int b, int c, int d);

void generate_and_check()
{
    FILE *file = fopen("collection_with_solution.txt", "w");
    if (file == NULL)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL)); // Initialize random seed

    for (int i = 0; i < 1000; i++)
    {                            // Generate 1000 sets of random numbers
        int a = rand() % 10 + 1; // Generate a random number between 1 and 13
        int b = rand() % 10 + 1;
        int c = rand() % 10 + 1;
        int d = rand() % 10 + 1;

        if (has_solution(a, b, c, d))
        {                                               
            fprintf(file, "%d %d %d %d\n", a, b, c, d); // Write the solvable combination to the file
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