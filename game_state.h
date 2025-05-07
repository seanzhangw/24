#pragma once // pragmatic info
#include "vga_driver/vga16_graphics.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


/* ------------------------ BEGIN: Game State --------------------------------*/
typedef enum
{
    EASY,
    MEDIUM,
    HARD
} Difficulty;

typedef struct
{
    Difficulty difficultyLevel; // Easy, Medium, Hard
    // GameMode gameMode;
} Settings;

typedef struct
{
    bool player1CardsSlid;
    bool player2CardsSlid;
    bool player1Win;
    bool player2Win;
} sharedFlags;

// Define the game states
typedef enum
{
    START_MENU,
    GAME_PLAYING,
    GAME_OVER
} GameState;
/* ------------------------ END: Game State --------------------------------*/

/* --------------------------- BEGIN: Card State ---------------------------*/
typedef enum
{
    DEFAULT,
    HOVERED,
    SELECTED,
    USED,
    RESULT
} CardState;

typedef struct
{
    int value;
    const unsigned char *image;
    int x;
    int y;
    int destX;
    int destY;
    float flipProgress;
    CardState state;
} Card;

typedef enum
{
    SELECT_NUM1,
    SELECT_OP,
    SELECT_NUM2
} Stage;
/* ----------------------------- END: Card State -----------------------------*/

/* ------------------------ BEGIN: Player State ------------------------------*/
typedef struct
{
    GameState currentState;
    int nums[4];   // Array to hold the numbers
    Card cards[4]; // Array to hold the cards
    bool onLeft;   // True if player is on left side of screen
    int num1;
    int num2;
    char op;
    Stage opStage; // Where each player is when performing operation
    int playerNum;
} Player;

extern Player player1; // Player 1
extern Player player2; // Player 2
/* ------------------------ END: Player State --------------------------------*/

extern char operations[];

extern volatile bool stateTransition;

void transitionToState(Player *player, GameState newState);

void executeStep(Player *player);

void cardSelect(Player *player);

void sol_init();

void handle_card_select(Player *player, bool enterPressed, int index);

void resetLevel(Player *player);

void slideCards(Player *player);