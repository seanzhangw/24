#include <stdbool.h>
#include "hardware/timer.h"

// Maximum number of solutions
#define MAX_SIZE 100

// Declare the global solution array
extern int arrSol[MAX_SIZE][6];

// Function declarations
int (*array_solutions(int size))[6];
bool has_solution(int a, int b, int c, int d);