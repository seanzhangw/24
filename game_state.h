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
    const unsigned char *image;
    int x;
    int y;
    int destX;
    int destY;
    float flipProgress;
} Card;

typedef enum {
    SELECT_NUM1,
    SELECT_OP,
    SELECT_NUM2
  } stage;

// all data related to player and their display
typedef struct
{
    GameState currentState;
    int nums[4];         // Array to hold the numbers
    Card cards[4];       // Array to hold the cards
    bool inputAvailable; // Flag to indicate if input is available
    bool onLeft;         // Flag to indicate if the player is on the left side
    int cardsShown;      // Progress for flipping the card
    stage opStage;       // Where each player is when performing operation
} Player;

extern Player player1; // Player 1
extern Player player2; // Player 2

void transitionToState(Player *player, GameState newState);

void executeStep(Player *player);

void cardSelect(Player *player);

void sol_init();
