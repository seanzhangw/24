#include "vga_driver/vga16_graphics.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Define the game states
typedef enum
{
    START_MENU,
    GAME_PLAYING,
    GAME_OVER
} GameState;

typedef struct
{
    int value;
    int x;
    int y;
    int destX;
    int destY;
} Card;

extern GameState currentState;
extern int stateChange;

// global variables for the numbers
extern int nums[4];

void transitionToState(GameState newState);

void executeStep();

void sol_init();
