#include <stdio.h>
#include <stdlib.h>
#include "game_state.h"

// Initialize the current game state
GameState currentState = START_MENU;
int stateChange = 0;

int nums[4] = {-1, -1, -1, -1}; // Initialize numbers to -1

void _generateNumbers()
{
    // Generate and display four random numbers between 1 and 9
    // TODO: add logic for guaranteeing numbers w/ a solution here
    nums[0] = (rand() % 9) + 1;
    nums[1] = (rand() % 9) + 1;
    nums[2] = (rand() % 9) + 1;
    nums[3] = (rand() % 9) + 1;
    // nums[0] = 1;
    // nums[1] = 1;
    // nums[2] = 6;
    // nums[3] = 4;
}

void _drawNumbers()
{
    char buffer[10];
    for (int i = 0; i < 4; i++)
    {
        if (nums[i] != -1)
        {
            setTextColor2(WHITE, BLACK);
            setCursor(210 + (i * 80), 400);
            sprintf(buffer, "%d ", nums[i]);
            writeStringBig(buffer);
        }
        else
        {
            // blank out the number
            fillRect(210 + (i * 80), 400, 16, 32, BLACK);
        }
    }
}

void transitionToState(GameState newState)
{
    currentState = newState;
    stateChange = 1;

    switch (newState)
    {
    case START_MENU:
        // Clear the screen
        fillRect(0, 0, 640, 480, BLACK);
        printf("In Start Menu State\n\r");
        stateChange = 0;

        // 340 is the cetner of the screen. "Press any key to start" is 23 characters long.
        // 23 * 8 = 184. 184 /2 = 92. 340 - 92 = 248.
        setCursor(248, 200);
        setTextColor2(WHITE, BLACK);
        writeStringBig("Press any key to start");
        break;
    case GAME_PLAYING:
        printf("In Game Playing State\n\r");
        fillRect(0, 0, 640, 480, BLACK);
        _generateNumbers();
        _drawNumbers();

        break;
    case GAME_OVER:
        // Clear the screen and display "Game Over!"
        fillRect(0, 0, 640, 480, BLACK);
        setCursor(248, 200);
        setTextColor2(WHITE, BLACK);
        writeStringBig("Game Over!");

        break;
    }
}

void executeStep()
{
    // Execute the current game state logic
    switch (currentState)
    {
    case START_MENU:
        // Logic for start menu
        break;
    case GAME_PLAYING:

        // NOTE: calculating values currently happens in the serial listener thread, should it be moved to here?
        _drawNumbers(); // Update the numbers on the screen

        // Check if we have a solution
        int contains24 = 0;
        int allNumbersUsed = 1;
        for (int i = 0; i < 4; i++)
        {
            if (nums[i] == 24)
            {
                contains24 = 1;
                break;
            }
            else if (nums[i] != -1)
            {
                allNumbersUsed = 0;
            }
        }

        if (contains24 && allNumbersUsed)
        {
            transitionToState(GAME_OVER);
        }
        else if (allNumbersUsed)
        {
            transitionToState(GAME_OVER);
        }
        break;
    case GAME_OVER:
        // Logic for game over
        break;
    default:
        break;
    }
}