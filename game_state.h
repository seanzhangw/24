#pragma once
#include "vga_driver/vga16_graphics.h"
#include "hardware/clocks.h"
#include "hardware/sync.h"
#include "hardware/timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ------------------------ BEGIN: Game State --------------------------------*/
#define ROWS 2
#define COLS 3

#define START_GAME_ROW 2

typedef struct
{
    int x;
    int y;
    char color;
    int len;
    char *text;
} MenuIcon;

extern MenuIcon startMenuIcons[ROWS][COLS]; // Global variable for the start menu icons

typedef enum
{
    EASY,
    MEDIUM,
    HARD
} Difficulty;

typedef struct
{
    Difficulty difficultyLevel; // Easy, Medium, Hard
    int mins;
} Settings;

#define SPINLOCK_ID 0

typedef struct
{
    int curRow;
    int curCol;
    Settings settings;
} StartMenuState;

extern spin_lock_t *menuLock;

extern StartMenuState startMenuState; // Global variable for the start menu state

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
/* ----------------------------- BEGIN: Card State ---------------------------*/
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
    float value;
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
    OP_DEFAULT,
    OP_HOVERED,
    OP_SELECTED,
} operatorState;

typedef struct
{
    operatorState state;
    char op;
} Operator;

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
    float nums[4]; // Array to hold the numbers
    Card cards[4]; // Array to hold the cards
    Operator operator;
    bool onLeft; // True if player is on left side of screen
    float num1;
    float num2;
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

void handle_start_menu_input(bool enterPressed, int index);

void resetLevel(Player *player);

void skipLevel(Player *player);
