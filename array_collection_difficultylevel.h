#include <stdbool.h>

// Maximum number of solutions
#define MAX_SIZE 100

// Declare the global solution array
extern int arrSol[MAX_SIZE][5];

// Function declarations
int (*array_solutions(int size))[5];
bool has_solution(int a, int b, int c, int d);