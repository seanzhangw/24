#include "assets.h"
#include "game_state.h"
#include "array_collection_difficultylevel.h"
#include "input_handler.h"
#include "eeprom.h"
// from dma-demo.c
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/spi.h"

// #include "background.h" // none of these can be include, otherwise compile error: multyiple definition
// #include "bingo.h"
// #include "buzzer.h"
// #include "deal_cards.h"
// #include "final_victory.h"
// #include "flip_cards.h"

char operations[4] = {'+', '-', '/', '*'}; // Array of operators

StartMenuState startMenuState = {0, 0, {EASY, 1}, LB_oneMin}; // Global variable for the start menu state

spin_lock_t *menuLock; // Spinlock for the start menu
spin_lock_t *paramLock;

extern int data_chan;
extern int data_chan2;
extern int data_chan3;
extern int data_chan4;
extern unsigned short *DAC_data_background;
extern unsigned short *DAC_data_click;
extern unsigned short *DAC_data_deal;
extern unsigned short *DAC_data_flip;

#define PLAYER1_CARD0_X 110
#define PLAYER1_CARD0_Y 150
#define PLAYER1_CARD1_X 110
#define PLAYER1_CARD1_Y 350
#define PLAYER1_CARD2_X 5
#define PLAYER1_CARD2_Y 250
#define PLAYER1_CARD3_X 215
#define PLAYER1_CARD3_Y 250

#define CARD_X_OFFSET 315

#define BACKGROUND DARK_GREEN

Player player1 = {START_MENU, {-1, -1, -1, -1}, {0}, 0, {OP_DEFAULT, ' '}, 0, -1, -1, SELECT_NUM1, 1}; // Player 1
Player player2 = {START_MENU, {-1, -1, -1, -1}, {0}, 0, {OP_DEFAULT, ' '}, 0, -1, -1, SELECT_NUM1, 2}; // Player 2

GameFlags gameFlags = {false, false, false, false, 0, false}; // Shared flags for both players

volatile bool stateTransition = false;

repeating_timer_t timer;

MenuIcon startMenuIcons[ROWS][COLS] = {
    {{406, 130, WHITE, 4, "EASY"}, {470, 130, WHITE, 6, "MEDIUM"}, {550, 130, WHITE, 4, "HARD"}},
    {{406, 230, WHITE, 5, "60 s"}, {480, 230, WHITE, 5, "120 s"}, {550, 230, WHITE, 5, "180 s"}},
    {{406, 330, WHITE, 5, "60 s"}, {480, 330, WHITE, 5, "120 s"}, {550, 330, WHITE, 5, "180 s"}},
};

EndScreenRow p1EndScreenRows[30] = {0};
EndScreenRow p2EndScreenRows[30] = {0};

int p1EndScreenRowCount = 0;
int p2EndScreenRowCount = 0;

void sol_init()
{
    array_solutions(100);
}

void resetPlayer(Player *player)
{
    player->num1 = -1;
    player->num2 = -1;
    player->operator.op = ' ';
    player->operator.state = OP_DEFAULT;
    player->opStage = SELECT_NUM1;
    for (int i = 0; i < 4; i++)
    {
        player->nums[i] = -1;
        player->cards[i].state = DEFAULT;
        player->cards[i].flipProgress = 0.0; // reset flip progress
    }
}

void prepNewRound()
{
    gameFlags.player1CardsSlid = false; // Reset the flags for the new round
    gameFlags.player2CardsSlid = false;
    gameFlags.player1Win = false;
    gameFlags.player2Win = false;
    gameFlags.savedScores = false;

    player1.solved = 0; // Reset the solved count for both players
    player2.solved = 0;

    resetPlayer(&player1); // Reset player 1
    resetPlayer(&player2); // Reset player 2
}

void resetLevel(Player *player)
{
    // restore default card states
    player->opStage = SELECT_NUM1;
    player->num1 = -1;
    player->num2 = -1;
    player->operator.op = ' ';
    player->operator.state = OP_DEFAULT;

    for (int i = 0; i < 4; i++)
    {
        player->nums[i] = player->cards[i].value; // restore the numbers
        player->cards[i].state = DEFAULT;
        // draw cards
        pasteImage(player->cards[i].image, IMG_HEIGHT, IMG_WIDTH,
                   player->cards[i].x, player->cards[i].y, BACKGROUND);
    }

    // erase outlines
    for (int i = 0; i < 4; i++)
    {
        drawRect(player->cards[i].x - 2, player->cards[i].y - 2,
                 2 * IMG_WIDTH + 4, IMG_HEIGHT + 4, BACKGROUND); // draw a card outline
    }
    int offset = -5;
    // erase previous operator underlines
    drawHLine(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT + 20, 10, BACKGROUND);
    drawHLine(player->cards[0].x + IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4 + 15, 10, BACKGROUND);
    drawHLine(player->cards[0].x + IMG_WIDTH + IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4 + 15, 10, BACKGROUND);
    drawHLine(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 2 + 10, 10, BACKGROUND);
}

void generateNumbers(Player *player, int difficulty)
{
    int chosenIndex = 0;
    for (int i = 0; i < MAX_SIZE; i++)
    {
        if (arrSol[i][4] == difficulty)
        {
            chosenIndex = i;
            arrSol[i][4] = -1;                   // mark the solution as used
            player->numSolutions = arrSol[i][5]; // store the number of solutions
            break;
        }
    }
    for (int i = 0; i < 4; i++)
    {
        player->nums[i] = arrSol[chosenIndex][i];
    }

    for (int j = 0; j < 4; j++)
    {
        // assign the integer value for calculations
        player->cards[j].value = player->nums[j];
        int index = (int)(player->nums[j]);

        // Assign a random suit based on the card value
        switch (rand() % 4)
        {
        case 0:
            player->cards[j].image = spades[index - 1];
            break;
        case 1:
            player->cards[j].image = hearts[index - 1];
            break;
        case 2:
            player->cards[j].image = diamonds[index - 1];
            break;
        case 3:
            player->cards[j].image = clubs[index - 1];
            break;
        }
        // set initial position of the cards
        player->cards[j].x = 240;
        player->cards[j].y = 100;

        // set destination position of the cards
        int offset = (player->playerNum == 2) ? CARD_X_OFFSET : 0;

        switch (j)
        {
        case 0:
            player->cards[j].destX = offset + PLAYER1_CARD0_X;
            player->cards[j].destY = PLAYER1_CARD0_Y;
            break;
        case 2:
            player->cards[j].destX = offset + PLAYER1_CARD2_X;
            player->cards[j].destY = PLAYER1_CARD2_Y;
            break;
        case 3:
            player->cards[j].destX = offset + PLAYER1_CARD3_X;
            player->cards[j].destY = PLAYER1_CARD3_Y;
            break;
        case 1:
            player->cards[j].destX = offset + PLAYER1_CARD1_X;
            player->cards[j].destY = PLAYER1_CARD1_Y;
            break;
        }
    }
}

void skipLevel(Player *player, int difficulty)
{
    // restore default card states
    player->opStage = SELECT_NUM1;
    player->num1 = -1;
    player->num2 = -1;
    player->operator.op = ' ';
    player->operator.state = OP_DEFAULT;

    int chosenIndex = 0;
    for (int i = 0; i < MAX_SIZE; i++)
    {
        if (arrSol[i][4] == difficulty)
        {
            chosenIndex = i;
            arrSol[i][4] = -1; // mark the solution as used
            break;
        }
    }
    for (int i = 0; i < 4; i++)
    {
        player->nums[i] = arrSol[chosenIndex][i];
    }

    for (int j = 0; j < 4; j++)
    {
        // assign the integer value for calculations
        player->cards[j].value = player->nums[j];
        int index = (int)(player->nums[j]);

        // Assign a random suit based on the card value
        switch (rand() % 4)
        {
        case 0:
            player->cards[j].image = spades[index - 1];
            break;
        case 1:
            player->cards[j].image = hearts[index - 1];
            break;
        case 2:
            player->cards[j].image = diamonds[index - 1];
            break;
        case 3:
            player->cards[j].image = clubs[index - 1];
            break;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        player->nums[i] = player->cards[i].value; // restore the numbers
        player->cards[i].state = DEFAULT;

        // draw cards
        pasteImage(player->cards[i].image, IMG_HEIGHT, IMG_WIDTH,
                   player->cards[i].x, player->cards[i].y, BACKGROUND);
    }

    // erase outlines
    for (int i = 0; i < 4; i++)
    {
        drawRect(player->cards[i].x - 2, player->cards[i].y - 2,
                 2 * IMG_WIDTH + 4, IMG_HEIGHT + 4, BACKGROUND); // draw a card outline
    }
    int offset = -5;
    // erase previous operator underlines
    drawHLine(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT + 20, 10, BACKGROUND);
    drawHLine(player->cards[0].x + IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4 + 15, 10, BACKGROUND);
    drawHLine(player->cards[0].x + IMG_WIDTH + IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4 + 15, 10, BACKGROUND);
    drawHLine(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 2 + 10, 10, BACKGROUND);
}

void handle_card_select(Player *player, bool enterPressed, int index)
{
    // if enter is pressed, that means a number is selected
    if (enterPressed)
    {
        dma_channel_set_read_addr(data_chan2, DAC_data_click, false); // set new source address

        // start the control channel
        dma_start_channel_mask(1u << data_chan2);

        // check which stage of operation we are on
        switch (player->opStage)
        {
        case SELECT_NUM1:
            if (index == NEUTRAL)
            {
                for (int i = 0; i < 4; i++)
                {
                    if (player->cards[i].state == HOVERED)
                    {
                        index = i; // find the selected card
                        break;
                    }
                }
                if (index == NEUTRAL)
                    return;
            }
            else if (player->cards[index].state == SELECTED || player->cards[index].state == USED)
            {
                return; // if the card is already selected, do nothing
            }
            player->num1 = player->nums[index]; // remove the number from the array
            player->nums[index] = -1;
            player->cards[index].state = SELECTED; // mark the card as selected
            player->opStage = SELECT_OP;           // move to the next stage
            drawRect(player->cards[index].x - 2, player->cards[index].y - 2,
                     2 * IMG_WIDTH + 4, IMG_HEIGHT + 4, BLUE); // draw a card outline
            break;
        case SELECT_OP:
            if (index == NEUTRAL && player->operator.state != OP_HOVERED)
            {
                return;
            }
            if (player->operator.state != OP_HOVERED)
            {
                player->operator.op = operations[index];
            }
            player->operator.state = OP_SELECTED; // mark the operator as selected
            player->opStage = SELECT_NUM2;        // move to the next stage

            int offset = -5;

            switch (player->operator.op)
            {
            case '+':
                drawHLine(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT + 20, 10, BLUE);
                break;
            case '-':
                drawHLine(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 2 + 10, 10, BLUE);
                break;
            case '/':
                drawHLine(player->cards[0].x + IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4 + 15, 10, BLUE);
                break;
            case '*':
                drawHLine(player->cards[0].x + IMG_WIDTH + IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4 + 15, 10, BLUE);
                break;
            default:
                break;
            }
            break;
        case SELECT_NUM2:
            if (index == NEUTRAL)
            {
                for (int i = 0; i < 4; i++)
                {
                    if (player->cards[i].state == HOVERED)
                    {
                        index = i; // find the selected card
                        break;
                    }
                }
                if (index == NEUTRAL)
                    return;
            }
            else if (player->cards[index].state == SELECTED || player->cards[index].state == USED)
            {
                return; // if the card is already selected, do nothing
            }
            player->num2 = player->nums[index]; // remove the number from the array
            player->nums[index] = -1;           // remove the number from the array

            // NOTE: this case handles
            // 1) performing the operation on the selected numbers
            // 2) erasing the selected cards and drawing the result
            // 3) checking if 24 has been made
            // perform the operation
            switch (player->operator.op)
            {
            case '+':
                player->nums[index] = player->num1 + player->num2; // add the two numbers
                break;
            case '-':
                player->nums[index] = player->num1 - player->num2; // subtract the two numbers
                break;
            case '*':
                player->nums[index] = player->num1 * player->num2; // multiply the two numbers
                break;
            case '/':
                if (player->num2 != 0)
                {
                    player->nums[index] = player->num1 / player->num2; // divide the two numbers
                }
                else
                {
                    // TODO: handle division by zero gracefully
                }
                break;
            }

            // erase the card
            for (int i = 0; i < 4; i++)
            {
                if (i == index || player->cards[i].state == SELECTED)
                {
                    fillRect(player->cards[i].x - 2, player->cards[i].y - 2,
                             2 * IMG_WIDTH + 4, IMG_HEIGHT + 4, BACKGROUND); // erase the card
                    if (player->cards[i].state == SELECTED)
                        player->cards[i].state = USED;
                    else
                        player->cards[i].state = RESULT;
                }
            }
            offset = -5;
            // erase previous operator underlines
            drawHLine(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT + 20, 10, BACKGROUND);
            drawHLine(player->cards[0].x + IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4 + 15, 10, BACKGROUND);
            drawHLine(player->cards[0].x + IMG_WIDTH + IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4 + 15, 10, BACKGROUND);
            drawHLine(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 2 + 10, 10, BACKGROUND);

            char buffer[16];
            sprintf(buffer, "%.2f", player->nums[index]); // convert the number to a string
            setCursor(player->cards[index].x + IMG_WIDTH - 20, player->cards[index].y + IMG_HEIGHT / 2);
            setTextColor2(WHITE, BACKGROUND); // set the text color
            writeStringBig(buffer);           // write the result
            player->opStage = SELECT_NUM1;    // move to the first stage

            // check if the result is 24
            if (player->nums[index] == 24)
            {
                for (int i = 0; i < 4; i++)
                {
                    // if we have remaining cards, keep the gameplay going
                    if (i != index && (player->cards[i].state == SELECTED || player->cards[i].state == DEFAULT || player->cards[i].state == RESULT))
                    {
                        return;
                    }
                }
                player->solved += 1; // add the score based on the difficulty level

                EndScreenRow *row = (player->playerNum == 1) ? &p1EndScreenRows[p1EndScreenRowCount] : &p2EndScreenRows[p2EndScreenRowCount];
                for (int i = 0; i < 4; i++)
                {
                    row->cards[i] = player->cards[i]; // Make a copy of the player's cards
                }
                row->numSolutions = player->numSolutions;

                (player->playerNum == 1) ? p1EndScreenRowCount++ : p2EndScreenRowCount++;

                skipLevel(player, startMenuState.settings.difficultyLevel); // generate new numbers
            }
        }
    }
    // this case is the highlight card case
    else if (player->opStage != SELECT_OP)
    {
        for (int i = 0; i < 4; i++)
        {
            if (player->cards[i].state != SELECTED && player->cards[i].state != USED)
            {
                if (i != index)
                {
                    drawRect(player->cards[i].x - 2, player->cards[i].y - 2,
                             2 * IMG_WIDTH + 4, IMG_HEIGHT + 4, BACKGROUND); // erase the outline
                    player->cards[i].state = DEFAULT;                        // mark the card as default
                }
                else
                {
                    drawRect(player->cards[i].x - 2, player->cards[i].y - 2,
                             2 * IMG_WIDTH + 4, IMG_HEIGHT + 4, ORANGE); // draw the outline
                    player->cards[i].state = HOVERED;                    // mark the card as hovered
                }
            }
        }
    }
    else if (player->opStage == SELECT_OP)
    {
        int offset = -5;
        if (player->operator.op != operations[index])
        {
            // erase previous operator underlines
            drawHLine(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT + 20, 10, BACKGROUND);
            drawHLine(player->cards[0].x + IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4 + 15, 10, BACKGROUND);
            drawHLine(player->cards[0].x + IMG_WIDTH + IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4 + 15, 10, BACKGROUND);
            drawHLine(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 2 + 10, 10, BACKGROUND);
        }
        player->operator.state = OP_HOVERED;     // mark the operator as hovered
        player->operator.op = operations[index]; // set the operator

        switch (index)
        {
        case 0:
            drawHLine(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT + 20, 10, ORANGE);
            break;
        case 1:
            drawHLine(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 2 + 10, 10, ORANGE);
            break;
        case 2:
            drawHLine(player->cards[0].x + IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4 + 15, 10, ORANGE);
            break;
        case 3:
            drawHLine(player->cards[0].x + IMG_WIDTH + IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4 + 15, 10, ORANGE);
            break;
        default:
            break;
        }
    }
}

void handle_start_menu_input(bool enterPressed, int index)
{
    if (enterPressed)
    {
        dma_channel_set_read_addr(data_chan2, DAC_data_click, false); // set new source address

        // start the control channel
        dma_start_channel_mask(1u << data_chan2);
        // debug print

        if (startMenuState.curRow == 0)
        {
            MenuIcon icon = startMenuIcons[startMenuState.curRow][startMenuState.settings.difficultyLevel];
            drawHLine(icon.x,
                      icon.y + 16, icon.len * 8, BACKGROUND); // erase the underline
        }
        else if (startMenuState.curRow == 1)
        {
            // subtract 1 here b/c we incremented 1 for an easier conversion from mins to seconds
            MenuIcon icon = startMenuIcons[startMenuState.curRow][startMenuState.settings.mins - 1];
            drawHLine(icon.x,
                      icon.y + 16, icon.len * 8, BACKGROUND); // erase the underline
        }
        else
        {
            MenuIcon icon = startMenuIcons[startMenuState.curRow][startMenuState.LBTime + 1];
            drawHLine(icon.x,
                      icon.y + 16, icon.len * 8, BACKGROUND); // erase the underline
        }
        MenuIcon icon = startMenuIcons[startMenuState.curRow][startMenuState.curCol];

        drawHLine(icon.x,
                  icon.y + 16, icon.len * 8, RED); // add an underline

        if (startMenuState.curRow == 0)
        {
            startMenuState.settings.difficultyLevel = startMenuState.curCol; // set the difficulty level
        }
        else if (startMenuState.curRow == 1)
        {
            startMenuState.settings.mins = startMenuState.curCol + 1; // set the time limit
        }
        else if (startMenuState.curRow == 2)
        {
            if (startMenuState.curCol == 0)
            {
                transitionToState(&player1, oneMin); // transition to leaderboard
            }
            if (startMenuState.curCol == 1)
            {
                transitionToState(&player1, twoMin); // transition to leaderboard
            }
            if (startMenuState.curCol == 2)
            {
                transitionToState(&player1, threeMin); // transition to leaderboard
            }
        }
        else
        {
            transitionToState(&player1, GAME_PLAYING); // transition to game playing state
        }
    }
    else
    {
        // update curRow and curCol based on joystick input
        if (index == NEUTRAL)
        {
            return; // no selection made
        }
        int rowChange[] = {-1, 1, 0, 0};
        int colChange[] = {0, 0, -1, 1};

        if (index >= 0 && index < 4)
        {
            int newRow = startMenuState.curRow + rowChange[index];
            int newCol = startMenuState.curCol + colChange[index];

            if (newRow >= 0 && (newRow < ROWS || newRow == START_GAME_ROW))
            {
                startMenuState.curRow = newRow;
            }
            if (newCol >= 0 && newCol < COLS)
            {
                startMenuState.curCol = newCol;
            }
        }
    }
}

void slideCards(Player *player)
{
    bool allCardsSlid = true;

    for (int i = 0; i < 4; i++)
    {
        // Check if the card is not already at its destination
        if (player->cards[i].x != player->cards[i].destX || player->cards[i].y != player->cards[i].destY)
        {
            // start the control channel
            dma_start_channel_mask(1u << data_chan);
            // debug print
            // printf("DMA channel %d started for deal\n", data_chan);

            int dx = player->cards[i].destX - player->cards[i].x;
            int dy = player->cards[i].destY - player->cards[i].y;

            int stepX = dx / 6; // Speed decreases as the card gets closer
            int stepY = dy / 6;

            if (stepX == 0 && dx != 0)
            {
                stepX = (dx > 0) ? 1 : -1; // Ensure at least 1 pixel movement
            }

            if (stepY == 0 && dy != 0)
            {
                stepY = (dy > 0) ? 1 : -1; // Ensure at least 1 pixel movement
            }

            moveImage((const unsigned char *)backOfCard, IMG_HEIGHT, IMG_WIDTH,
                      player->cards[i].x,
                      player->cards[i].y,
                      player->cards[i].x + stepX,
                      player->cards[i].y + stepY,
                      BACKGROUND);
            player->cards[i].x += stepX;
            player->cards[i].y += stepY;
            allCardsSlid = false;
        }
    }
    if (allCardsSlid)
    {
        if (player->playerNum == 1)
        {
            gameFlags.player1CardsSlid = true;
        }
        else
        {
            gameFlags.player2CardsSlid = true;
        }
        // dma_channel_set_read_addr(data_chan, DAC_data_flip, false); // set new source address
        // dma_channel_set_trans_count(data_chan, 6880, false); // set new length ^
    }
}

void flipCards(Player *player)
{

    for (int i = 0; i < 4; i++)
    {
        if (player->cards[0].flipProgress == 0)
        {
            // start the control channel
            dma_start_channel_mask(1u << data_chan4);
            // debug print
            // printf("DMA channel %d started for flip\n", data_chan4);
        }

        if (player->cards[i].flipProgress <= 1.0 + 0.01)
        {
            flipImage((const unsigned char *)backOfCard, IMG_HEIGHT, IMG_WIDTH,
                      player->cards[i].x, player->cards[i].y,
                      (const unsigned char *)player->cards[i].image, player->cards[i].flipProgress, BACKGROUND);

            player->cards[i].flipProgress += 0.1; // Increment the flip progress
        }
    }
}

void drawDeck()
{
    pasteImage((const unsigned char *)backOfCard, IMG_HEIGHT, IMG_WIDTH,
               270, 10, BACKGROUND); // Draw the back of the card
    drawHLine(270, 130, 100, WHITE);
    drawHLine(270, 131, 100, DARK_BLUE);
    drawHLine(270, 132, 100, WHITE);
    drawHLine(270, 133, 100, DARK_BLUE);
    drawHLine(270, 134, 100, WHITE);
    drawHLine(270, 135, 100, DARK_BLUE);
    drawHLine(270, 136, 100, WHITE);
    drawHLine(270, 137, 100, DARK_BLUE);
}

void drawStartMenu()
{
    // draw the logo
    pasteImage((const unsigned char *)logo, LOGO_HEIGHT, LOGO_WIDTH,
               30, 80, BACKGROUND);

    setTextColorBig(RED, BACKGROUND);

    // difficulty select
    setCursor(450, 100);
    writeStringBig("Difficulty: ");

    // time limit select
    setCursor(470, 200);
    writeStringBig("Time: ");

    // leader board select
    setCursor(440, 300);
    writeStringBig("Leaderboard: ");

    setTextColorBig(RED, BACKGROUND);
    setCursor(450, 420);
    writeStringBig("Start Game");

    setTextColorBig(WHITE, BACKGROUND);

    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            setCursor(startMenuIcons[i][j].x, startMenuIcons[i][j].y);
            writeStringBig(startMenuIcons[i][j].text);
        }
    }

    drawHLine(startMenuIcons[0][startMenuState.settings.difficultyLevel].x,
              startMenuIcons[0][startMenuState.settings.difficultyLevel].y + 16,
              startMenuIcons[0][startMenuState.settings.difficultyLevel].len * 8, RED); // add an underline
    drawHLine(startMenuIcons[1][startMenuState.settings.mins - 1].x,
              startMenuIcons[1][startMenuState.settings.mins - 1].y + 16,
              startMenuIcons[1][startMenuState.settings.mins - 1].len * 8, RED); // add an underline
}

void drawLeaderboard_60s()
{
    fillRect(0, 0, 640, 480, BACKGROUND);

    setTextColorBig(RED, BACKGROUND);
    setCursor(250, 20);
    writeStringBig("LEADERBOARD - 60s");

    char buffer[32];
    char name[NAME_LEN + 1];
    int baseY = 60;

    for (int row = 0; row < MAX_DISPLAY; row++)
    {
        int y = baseY + row * 40;

        // Read from EEPROM
        uint16_t addr = get_entry_addr(0, row); // mode 0 = 60s
        eeprom_read_name(addr, name);
        name[NAME_LEN] = '\0';
        uint16_t score = eeprom_read_uint16(addr + NAME_LEN);
        printf("Name: %s, Score: %d\n", name, score);

        // Draw entry background stripe
        fillRect(60, y, 520, 30, (row % 2 == 0) ? DARK_BLUE : DARK_GREEN);

        setCursor(80, y + 5);
        setTextColorBig(WHITE, (row % 2 == 0) ? DARK_BLUE : DARK_GREEN);

        if (score == 0xFFFF)
        {
            sprintf(buffer, "%2d. ------", row + 1);
        }
        else
        {
            sprintf(buffer, "%2d. %-8s - %d", row + 1, name, score);
        }
        writeStringBig(buffer);
    }

    // Exit text
    setCursor(250, 440);
    setTextColorBig(RED, BACKGROUND);
    writeStringBig("Press Enter to Exit");
}

void drawLeaderboard_120s()
{
    fillRect(0, 0, 640, 480, BACKGROUND);

    setTextColorBig(RED, BACKGROUND);
    setCursor(250, 20);
    writeStringBig("LEADERBOARD - 120s");

    char buffer[32];
    char name[NAME_LEN + 1];
    int baseY = 60;

    for (int row = 0; row < MAX_DISPLAY; row++)
    {
        int y = baseY + row * 40;

        // Read from EEPROM
        uint16_t addr = get_entry_addr(1, row);
        eeprom_read_name(addr, name);
        name[NAME_LEN] = '\0';
        uint16_t score = eeprom_read_uint16(addr + NAME_LEN);

        // Draw entry background stripe
        fillRect(60, y, 520, 30, (row % 2 == 0) ? DARK_BLUE : DARK_GREEN);

        setCursor(80, y + 5);
        setTextColorBig(WHITE, (row % 2 == 0) ? DARK_BLUE : DARK_GREEN);

        if (score == 0xFFFF)
        {
            sprintf(buffer, "%2d. ------", row + 1);
        }
        else
        {
            sprintf(buffer, "%2d. %-8s - %d", row + 1, name, score);
        }
        writeStringBig(buffer);
    }

    // Exit text
    setCursor(250, 440);
    setTextColorBig(RED, BACKGROUND);
    writeStringBig("Press Enter to Exit");
}

void drawLeaderboard_180s()
{
    fillRect(0, 0, 640, 480, BACKGROUND);

    setTextColorBig(RED, BACKGROUND);
    setCursor(250, 20);
    writeStringBig("LEADERBOARD - 180s");

    char buffer[32];
    char name[NAME_LEN + 1];
    int baseY = 60;

    for (int row = 0; row < MAX_DISPLAY; row++)
    {
        int y = baseY + row * 40;

        // Read from EEPROM
        uint16_t addr = get_entry_addr(2, row);
        eeprom_read_name(addr, name);
        name[NAME_LEN] = '\0';
        uint16_t score = eeprom_read_uint16(addr + NAME_LEN);

        // Draw entry background stripe
        fillRect(60, y, 520, 30, (row % 2 == 0) ? DARK_BLUE : DARK_GREEN);

        setCursor(80, y + 5);
        setTextColorBig(WHITE, (row % 2 == 0) ? DARK_BLUE : DARK_GREEN);

        if (score == 0xFFFF)
        {
            sprintf(buffer, "%2d. ------", row + 1);
        }
        else
        {
            sprintf(buffer, "%2d. %-8s - %d", row + 1, name, score);
        }
        writeStringBig(buffer);
    }

    // Exit text
    setCursor(250, 440);
    setTextColorBig(RED, BACKGROUND);
    writeStringBig("Press Enter to Exit");
}

// void serialInput(uint16_t score1, uint16_t score2, int mode)
// {
//     char name_buf[NAME_LEN + 1];

//     // --- Player 1 ---
//     pt_add_thread(leaderboard_input);
// }
// printf("Enter Player 1 name (max %d chars): ", NAME_LEN);
// fflush(stdout);

// memset(name_buf, 0, sizeof(name_buf));

// if (!fgets(name_buf, sizeof(name_buf), stdin))
// {
//     printf("Failed to read input.\n");
//     continue;
// }

// size_t len = strlen(name_buf);
// if (len > 0 && name_buf[len - 1] == '\n')
// {
//     name_buf[len - 1] = '\0';
//     len--;
// }
// else
// {
//     // Flush the rest of the line if input was too long
//     int ch;
//     while ((ch = getchar()) != '\n' && ch != EOF)
//         ;
// }
// printf("Length = %zu\n", len);

// if (len > 0 && len <= NAME_LEN)
// {
//     insert_score(name_buf, score1, mode);
//     printf("Player 1 score saved as %s: %d\n", name_buf, score1);
//     break;
// }
// else
// {
//     printf("Invalid name length. Please enter 1 to %d characters.\n", NAME_LEN);
// }

// --- Player 2 ---
// while (1)
// {
// printf("Enter Player 2 name (max %d chars): ", NAME_LEN);
// fflush(stdout);

// memset(name_buf, 0, sizeof(name_buf));

// if (!fgets(name_buf, sizeof(name_buf), stdin))
// {
//     printf("Failed to read input.\n");
//     continue;
// }

// size_t len = strlen(name_buf);
// if (len > 0 && name_buf[len - 1] == '\n')
// {
//     name_buf[len - 1] = '\0';
//     len--;
// }

// if (len > 0 && len <= NAME_LEN)
// {
//     insert_score(name_buf, score1, mode);
//     printf("Player 2 score saved as %s: %d\n", name_buf, score2);
//     break;
// }
// else
// {
//     printf("Invalid name length. Please enter 1 to %d characters.\n", NAME_LEN);
// }
// }

void animateStartMenu()
{
    if (startMenuState.curRow == START_GAME_ROW)
    {
        // blink the start game text
        if ((time_us_32() >> 18) & 1)
        {
            fillRect(450, 420, 100, 15, BACKGROUND);
        }
        else
        {
            setTextColorBig(RED, BACKGROUND);
            setCursor(450, 420);
            writeStringBig("Start Game");
        }
    }
    else
    {
        setTextColorBig(RED, BACKGROUND);
        setCursor(450, 420);
        writeStringBig("Start Game");
    }
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            // blink the text if i == curRow && j == curCol
            if (i == startMenuState.curRow && j == startMenuState.curCol)
            {
                if ((time_us_32() >> 18) & 1)
                {
                    fillRect(startMenuIcons[i][j].x, startMenuIcons[i][j].y, 50, 15, BACKGROUND);
                }
                else
                {
                    setTextColorBig(startMenuIcons[i][j].color, BACKGROUND);
                    setCursor(startMenuIcons[i][j].x, startMenuIcons[i][j].y);
                    writeStringBig(startMenuIcons[i][j].text);
                }
            }
            else
            {
                setTextColorBig(startMenuIcons[i][j].color, BACKGROUND);
                setCursor(startMenuIcons[i][j].x, startMenuIcons[i][j].y);
                writeStringBig(startMenuIcons[i][j].text);
            }
        }
    }
}

void drawOperators(Player *player)
{
    int offset = -5;
    // Draw the operators
    drawCharBig(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT - offset, '+', WHITE, BACKGROUND);
    drawCharBig(player->cards[0].x + IMG_WIDTH - IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4, '/', WHITE, BACKGROUND);
    drawCharBig(player->cards[0].x + IMG_WIDTH + IMG_WIDTH / 2 + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 4, '*', WHITE, BACKGROUND);
    drawCharBig(player->cards[0].x + IMG_WIDTH + offset, player->cards[0].y + IMG_HEIGHT + IMG_HEIGHT / 2 + offset, '-', WHITE, BACKGROUND);
}

void drawRoundParams()
{
    setTextColor2(RED, BACKGROUND);
    setCursor(5, 5);
    writeStringBig("P1 Score: ");
    setCursor(5, 25);
    writeStringBig("P2 Score: ");
    setCursor(5, 45);
    writeStringBig("Time Left: ");
    setCursor(5, 65);
    writeStringBig("Difficulty: ");
    setTextColor2(WHITE, BACKGROUND);
    setCursor(100, 65);
    writeStringBig(startMenuIcons[0][startMenuState.settings.difficultyLevel].text); // write the difficulty level
}

void updateParams(Player *player)
{
    char buffer[16];
    if (player->playerNum == 1)
        setCursor(100, 5);
    else
        setCursor(100, 25);

    sprintf(buffer, "%d", player->solved * (startMenuState.settings.difficultyLevel + 1)); // convert the score to a string
    setTextColor2(WHITE, BACKGROUND);                                                      // set the text color
    writeStringBig(buffer);                                                                // write the score

    setCursor(100, 45);
    sprintf(buffer, "%d ", gameFlags.secondsLeft); // convert the time to a string
    writeStringBig(buffer);                        // write the time
}

bool timer_callback(repeating_timer_t *rt)
{
    if (gameFlags.secondsLeft > 0)
        gameFlags.secondsLeft--;
    else
        return false; // Stop timer when timeLeft reaches 0
    return true;
}
void prepGameOverScreen()
{
    for (int i = 0; i < p1EndScreenRowCount; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            p1EndScreenRows[i].cards[j].x = 0;
            p1EndScreenRows[i].cards[j].y = 20 + i * (IMG_HEIGHT + 10);
            p1EndScreenRows[i].cards[j].destX = 50 + j * (IMG_WIDTH * 2 + 10);
            p1EndScreenRows[i].cards[j].destY = p1EndScreenRows[i].cards[j].y;
        }
        p1EndScreenRows[i].progress = 0.;
    }

    for (int i = 0; i < p2EndScreenRowCount; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            p2EndScreenRows[i].cards[j].x = 640;
            p2EndScreenRows[i].cards[j].y = 20 + i * (IMG_HEIGHT + 10);
            p2EndScreenRows[i].cards[j].destX = 340 + j * (IMG_WIDTH * 2 + 10);
            p2EndScreenRows[i].cards[j].destY = p2EndScreenRows[i].cards[j].y;
        }
        p2EndScreenRows[i].progress = 0.;
    }
}

void drawGameOver()
{
    fillRect(0, 0, 640, 480, BACKGROUND);

    // Title banner
    fillRect(120, 40, 400, 60, CYAN);
    setTextColorBig(YELLOW, CYAN);
    setCursor(275, 60);
    writeStringBig("GAME OVER!");

    // Player 1 Score Box
    fillRect(60, 140, 240, 100, BLUE);
    drawRect(60, 140, 240, 100, WHITE); // border
    setCursor(80, 160);
    setTextColorBig(WHITE, BLUE);
    char buffer1[32];
    int score1 = player1.solved * (startMenuState.settings.difficultyLevel + 1);
    sprintf(buffer1, "Player 1:");
    writeStringBig(buffer1);
    setCursor(80, 190);
    sprintf(buffer1, "Score: %d", score1);
    writeStringBig(buffer1);

    // Player 2 Score Box
    fillRect(340, 140, 240, 100, RED);
    drawRect(340, 140, 240, 100, WHITE); // border
    setCursor(360, 160);
    setTextColorBig(WHITE, RED);
    char buffer2[32];
    int score2 = player2.solved * (startMenuState.settings.difficultyLevel + 1);
    sprintf(buffer2, "Player 2:");
    writeStringBig(buffer2);
    setCursor(360, 190);
    sprintf(buffer2, "Score: %d", score2);
    writeStringBig(buffer2);

    // Instructions
    setTextColorBig(RED, BACKGROUND);
    setCursor(220, 300);
    writeStringBig("Press ENTER to Save Scores");

    setCursor(210, 350);
    writeStringBig("Press RESET to Return to Menu");

    // player1.solved = 0;
    // player2.solved = 0;
}

void transitionToState(Player *player, GameState newState)
{
    stateTransition = true; // Set the state transition flag
    switch (newState)
    {
    case START_MENU:
        // Clear the screen
        fillRect(0, 0, 640, 480, BACKGROUND);
        dma_channel_set_read_addr(data_chan, DAC_data_deal, false);  // set new source address
        dma_channel_set_read_addr(data_chan4, DAC_data_flip, false); // set new source address
        prepNewRound();
        drawStartMenu();
        player1.currentState = START_MENU;
        player2.currentState = START_MENU;
        break;
    case oneMin:
        fillRect(0, 0, 640, 480, BACKGROUND);
        player1.currentState = oneMin;
        player2.currentState = oneMin;
        drawLeaderboard_60s();
        break;
    case twoMin:
        fillRect(0, 0, 640, 480, BACKGROUND);
        player1.currentState = twoMin;
        player2.currentState = twoMin;
        drawLeaderboard_120s();
        break;
    case threeMin:
        fillRect(0, 0, 640, 480, BACKGROUND);
        player1.currentState = threeMin;
        player2.currentState = threeMin;
        drawLeaderboard_180s();
        break;
    case GAME_PLAYING: //!!!
        // printf("In Game Playing State\n\r");
        fillRect(0, 0, 640, 480, BACKGROUND);
        drawRoundParams();
        // import start menu settings
        generateNumbers(&player1, startMenuState.settings.difficultyLevel);
        generateNumbers(&player2, startMenuState.settings.difficultyLevel);
        gameFlags.secondsLeft = startMenuState.settings.mins * 60; // Set the time limit
        player1.currentState = GAME_PLAYING;
        player2.currentState = GAME_PLAYING;

        // timer interrupt for countdown
        if (player1.playerNum == 1)
            add_repeating_timer_us(-1000000, timer_callback, NULL, &timer);
        break;
    case GAME_OVER:
        // Clear the screen and display "Game Over!"
        drawGameOver();
        // prepGameOverScreen();
        player1.currentState = GAME_OVER;
        player2.currentState = GAME_OVER;
        // generate background music
        // dma_start_channel_mask(1u << data_chan3);
        // dma_channel_set_read_addr(data_chan3, DAC_data_background, false); // set new source address
        // debug print
        // printf("DMA channel %d started for background\n", data_chan3);
        // serialInput(player1.score, player2.score, startMenuState.settings.mins - 1);
        break;
    }
    // Reset the state transition flag
    stateTransition = false;
}

void executeStep(Player *player)
{
    while (stateTransition)
        ;

    // Execute the current game state logic for the given player
    switch (player->currentState)
    {
    case START_MENU:
        // lock
        uint32_t irq_status = spin_lock_blocking(menuLock);
        animateStartMenu();
        // unlock
        spin_unlock(menuLock, irq_status);
        break;

    case oneMin:
        break;
    case twoMin:
        break;
    case threeMin:
        break;
    case GAME_PLAYING:
        drawDeck();

        slideCards(player); // Slide the cards
        if (gameFlags.player1CardsSlid && gameFlags.player2CardsSlid)
        {
            flipCards(player);
            drawOperators(player);
        }

        // lock
        uint32_t param_irq_status = spin_lock_blocking(paramLock);
        updateParams(player);
        // unlock
        spin_unlock(paramLock, param_irq_status);

        if (gameFlags.secondsLeft <= 0)
        {
            cancel_repeating_timer(&timer);
            transitionToState(player, GAME_OVER); // Transition to game over state
        }
        break;
    case GAME_OVER:
        // for (int k = 0; k < p1EndScreenRowCount; k++)
        // {
        //     EndScreenRow *player = &p1EndScreenRows[k];
        //     for (int i = 0; i < 4; i++)
        //     {
        //         // Check if the card is not already at its destination
        //         if (player->cards[i].x != player->cards[i].destX || player->cards[i].y != player->cards[i].destY)
        //         {
        //             int dx = player->cards[i].destX - player->cards[i].x;
        //             int dy = player->cards[i].destY - player->cards[i].y;

        //             int stepX = dx / 6; // Speed decreases as the card gets closer
        //             int stepY = dy / 6;

        //             if (stepX == 0 && dx != 0)
        //             {
        //                 stepX = (dx > 0) ? 1 : -1; // Ensure at least 1 pixel movement
        //             }

        //             if (stepY == 0 && dy != 0)
        //             {
        //                 stepY = (dy > 0) ? 1 : -1; // Ensure at least 1 pixel movement
        //             }

        //             moveImage((const unsigned char *)backOfCard, IMG_HEIGHT, IMG_WIDTH,
        //                       player->cards[i].x,
        //                       player->cards[i].y,
        //                       player->cards[i].x + stepX,
        //                       player->cards[i].y + stepY,
        //                       BACKGROUND);
        //             player->cards[i].x += stepX;
        //             player->cards[i].y += stepY;
        //         }
        //     }
        // }
        break;
    default:
        break;
    }
}