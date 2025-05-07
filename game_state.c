#include "assets.h"

#include "game_state.h"
#include "array_collection_difficultylevel.h"
#include "input_handler.h"
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

char operations[] = {'+', '-', '*', '/'};

extern int data_chan;
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

Player player1 = {START_MENU, {-1, -1, -1, -1}, {0}, true, -1, -1, ' ', SELECT_NUM1, 1};  // Player 1
Player player2 = {START_MENU, {-1, -1, -1, -1}, {0}, false, -1, -1, ' ', SELECT_NUM1, 2}; // Player 2

sharedFlags gameFlags = {false, false, false, false}; // Shared flags for both players

volatile bool stateTransition = false;

void sol_init()
{
    array_solutions(100);
}

void resetPlayer(Player *player)
{
    player->num1 = -1;
    player->num2 = -1;
    player->op = ' ';
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

    resetPlayer(&player1); // Reset player 1
    resetPlayer(&player2); // Reset player 2
}

void resetLevel(Player *player)
{
    // restore default card states
    player->opStage = SELECT_NUM1;
    player->num1 = -1;
    player->num2 = -1;
    player->op = ' ';

    for (int i = 0; i < 4; i++)
    {
        player->nums[i] = player->cards[i].value; // restore the numbers
        player->cards[i].state = DEFAULT;
        // draw cards
        pasteImage(player->cards[i].image, IMG_HEIGHT, IMG_WIDTH,
                   player->cards[i].x, player->cards[i].y);
    }

    // erase outlines
    for (int i = 0; i < 4; i++)
    {
        drawRect(player->cards[i].x - 2, player->cards[i].y - 2,
                 2 * IMG_WIDTH + 4, IMG_HEIGHT + 4, BLACK); // draw a card outline
    }
}

void handle_card_select(Player *player, bool enterPressed, int index)
{
    // if enter is pressed, that means a number is selected
    if (enterPressed)
    {
        // check which stage of operation we are on
        switch (player->opStage)
        {
        case SELECT_NUM1:
            if (index == -1)
            {
                for (int i = 0; i < 4; i++)
                {
                    if (player->cards[i].state == HOVERED)
                    {
                        index = i; // find the selected card
                        break;
                    }
                }
                if (index == -1)
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
            if (index == -1)
            {
                return;
            }
            player->op = operations[index]; // remove the number from the array
            player->opStage = SELECT_NUM2;  // move to the next stage
            drawCharBig(PLAYER1_CARD0_X + IMG_WIDTH, PLAYER1_CARD2_Y + IMG_HEIGHT / 2, operations[index], WHITE, BLACK);
            break;
        case SELECT_NUM2:
            if (index == -1)
            {
                for (int i = 0; i < 4; i++)
                {
                    if (player->cards[i].state == HOVERED)
                    {
                        index = i; // find the selected card
                        break;
                    }
                }
                if (index == -1)
                    return;
            }
            player->num2 = player->nums[index]; // remove the number from the array
            player->nums[index] = -1;           // remove the number from the array

            // NOTE: this case handles
            // 1) performing the operation on the selected numbers
            // 2) erasing the selected cards and drawing the result
            // 3) checking if 24 has been made
            // perform the operation
            switch (player->op)
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
                             2 * IMG_WIDTH + 4, IMG_HEIGHT + 4, BLACK); // erase the card
                    if (player->cards[i].state == SELECTED)
                        player->cards[i].state = USED;
                    else
                        player->cards[i].state = RESULT;
                }
            }
            // erase the operator
            fillRect(PLAYER1_CARD0_X + IMG_WIDTH, PLAYER1_CARD2_Y + IMG_HEIGHT / 2, 10, 10, BLACK);

            char buffer[2];
            sprintf(buffer, "%d", player->nums[index]); // convert the number to a string
            setCursor(player->cards[index].x + IMG_WIDTH, player->cards[index].y + IMG_HEIGHT / 2);
            setTextColor2(WHITE, BLACK);   // set the text color
            writeStringBig(buffer);        // write the result
            player->opStage = SELECT_NUM1; // move to the first stage

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
                // if we have no remaining cards, we have a winner
                transitionToState(player, GAME_OVER); // transition to game over state
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
                             2 * IMG_WIDTH + 4, IMG_HEIGHT + 4, BLACK); // erase the outline
                    player->cards[i].state = DEFAULT;                   // mark the card as default
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
}

void generateNumbers(Player *player)
{
    srand(time_us_32()); // Seed the random number generator with the current time
    for (int i = 0; i < 4; i++)
    {
        player->nums[i] = arrSol[rand() % 100][i];
    }

    for (int j = 0; j < 4; j++)
    {
        // assign the integer value for calculations
        player->cards[j].value = player->nums[j];

        // Assign a random suit based on the card value
        switch (rand() % 4)
        {
        case 0:
            player->cards[j].image = spades[player->nums[j] - 1];
            break;
        case 1:
            player->cards[j].image = hearts[player->nums[j] - 1];
            break;
        case 2:
            player->cards[j].image = diamonds[player->nums[j] - 1];
            break;
        case 3:
            player->cards[j].image = clubs[player->nums[j] - 1];
            break;
        }
        // set initial position of the cards
        player->cards[j].x = 240;
        player->cards[j].y = 100;

        // set destination position of the cards
        int offset = 0;
        if (!player->onLeft)
        {
            offset = 315;
        }
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

void slideCards(Player *player)
{
    bool allCardsSlid = true;

    for (int i = 0; i < 4; i++)
    {
        // Check if the card is not already at its destination
        if (player->cards[i].x != player->cards[i].destX || player->cards[i].y != player->cards[i].destY)
        {        
            dma_start_channel_mask(1u << data_chan) ;

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
                      player->cards[i].y + stepY);
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
    }
}

void flipCards(Player *player)
{   
    // dma_channel_set_read_addr(data_chan, DAC_data_flip, false);  // 设置新的源地址
    // // dma_channel_set_trans_count(data_chan, flip_cards_audio_len, false); // 设置新长度
    // dma_channel_set_trans_count(data_chan, 6880, false); // 设置新长度
    // // dma_channel_start(data_chan);  // 启动 DMA 传输
    // // start the control channel
    // dma_start_channel_mask(1u << data_chan) ;
    // // debug print
    // printf("actual DMA channel %d\n", data_chan);
    // printf("DMA started\n");
    
    for (int i = 0; i < 4; i++)
    {
        if (player->cards[0].flipProgress == 0) {
            dma_channel_set_read_addr(data_chan, DAC_data_flip, false);  // set new source address
            // dma_channel_set_trans_count(data_chan, flip_cards_audio_len, false); // set new length
            dma_channel_set_trans_count(data_chan, 6880, false); // set new length ^
            // dma_channel_start(data_chan); //another function has the same functionality
            // start the control channel
            dma_start_channel_mask(1u << data_chan) ;
        }

        if (player->cards[i].flipProgress <= 1.0 + 0.01)
        {
            flipImage((const unsigned char *)backOfCard, IMG_HEIGHT, IMG_WIDTH,
                      player->cards[i].x, player->cards[i].y,
                      (const unsigned char *)player->cards[i].image, player->cards[i].flipProgress);

            player->cards[i].flipProgress += 0.1; // Increment the flip progress
        }  
    }
}

void drawDeck()
{
    pasteImage((const unsigned char *)backOfCard, IMG_HEIGHT, IMG_WIDTH,
               270, 10); // Draw the back of the card
    drawHLine(270, 130, 100, WHITE);
    drawHLine(270, 131, 100, DARK_BLUE);
    drawHLine(270, 132, 100, WHITE);
    drawHLine(270, 133, 100, DARK_BLUE);
    drawHLine(270, 134, 100, WHITE);
    drawHLine(270, 135, 100, DARK_BLUE);
    drawHLine(270, 136, 100, WHITE);
    drawHLine(270, 137, 100, DARK_BLUE);
}
void transitionToState(Player *player, GameState newState)
{
    stateTransition = true; // Set the state transition flag
    switch (newState)
    {
    case START_MENU:
        // Clear the screen
        fillRect(0, 0, 640, 480, BLACK);
        prepNewRound();
        player1.currentState = START_MENU;
        player2.currentState = START_MENU;
        break;
    case GAME_PLAYING: //!!!
        printf("In Game Playing State\n\r");
        fillRect(0, 0, 640, 480, BLACK);
        generateNumbers(&player1);
        generateNumbers(&player2);
        player1.currentState = GAME_PLAYING;
        player2.currentState = GAME_PLAYING;
        break;
    case GAME_OVER:
        // Clear the screen and display "Game Over!"
        fillRect(0, 0, 640, 480, BLACK);
        setCursor(248, 200);
        setTextColor2(WHITE, BLACK);
        writeStringBig("Game Over!");
        player1.currentState = GAME_OVER;
        player2.currentState = GAME_OVER;
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
        pasteImage((const unsigned char *)logo, LOGO_HEIGHT, LOGO_WIDTH,
                   30, 80); // Draw the logo

        // // Blink the text NOTE: base x position on the selection
        // if ((time_us_32() / 500000) % 2 == 0)
        // {
        //     fillRect(248, 200, 400, 50, BLACK);
        // }

        // setCursor(340, 200);
        // setTextColor2(WHITE, BLACK);
        // writeStringBig("1 Player");
        break;
    case GAME_PLAYING:
        drawDeck();

        slideCards(player); // Slide the cards
        if (gameFlags.player1CardsSlid && gameFlags.player2CardsSlid)
        {
            flipCards(player);
        }
        break;
    case GAME_OVER:
        // Logic for game over
        break;
    default:
        break;
    }
}
