#include "vga_driver/vga16_graphics.h"

// Define the game states
typedef enum
{
    START_MENU,
    GAME_PLAYING,
    GAME_OVER
} GameState;

extern GameState currentState;
extern int stateChange;

// global variables for the numbers
extern int nums[4];

void transitionToState(GameState newState);

void executeStep();
