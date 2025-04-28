#include "assets.h"

#include "game_state.h"
#include "array_sol.h"
#include "input_handler.h"

Player player1 = {START_MENU, {-1, -1, -1, -1}, {0}, false, true, 3, SELECT_NUM1};  // Player 1
Player player2 = {START_MENU, {-1, -1, -1, -1}, {0}, false, false, 3, SELECT_NUM1}; // Player 2

int num1 = 0;
int num2 = 0;
char op = 0;


void sol_init()
{
    array_solutions(100);
}

void cardSelect(Player *player) {
    printf("current state: %d\n", player->currentState);
    int selected_index1 = -1;
    int selected_op = -1;
    int selected_index2 = -1;
    // intermidate variable
    switch (player->currentState)
    {
    case START_MENU:
        printf("in start menu\n");
        // Read button inputs
        if (gpio_get(BUTTON_PIN_P1_E) == 0 || gpio_get(BUTTON_PIN_P1_R) == 0)
        {
            while (gpio_get(BUTTON_PIN_P1_E) == 0 || gpio_get(BUTTON_PIN_P1_R) == 0);

            transitionToState(player, GAME_PLAYING);
            // current_stage = 0;
            // selected_index1 = -1;
            // selected_op = -1;
            // selected_index2 = -1;    

            num1 = 0;
            num2 = 0;
            op = ' ';
        }
        break;

    case GAME_PLAYING:
        printf("in game palying\n");
        // Read ADC Inputs
        adc_select_input(ADC_CHAN0);
        int joystick_x = adc_read();

        adc_select_input(ADC_CHAN1);
        int joystick_y = adc_read();

        // User Selection
        int index = joystickSelect(joystick_x, joystick_y);

        if (index != -1 && gpio_get(BUTTON_PIN_P1_E) == 0)
        {
            while (gpio_get(BUTTON_PIN_P1_E) == 0);

            printf("op stage: %d\n", player->opStage);

            if (player->opStage == SELECT_NUM1) {
                selected_index1 = index;
                num1 = player->nums[index];
                player->opStage = SELECT_OP;
                printf("num1: %d\n", num1);
            } else if (player->opStage == SELECT_OP) {
                selected_op = index;
                op = operations[index];
                player->opStage = SELECT_NUM2;
                printf("op: %c\n", op);
            } else if (player->opStage == SELECT_NUM2) {
                selected_index2 = index;
                num2 = player->nums[index];
                printf("num2: %d\n", num2);
            }

            // Perform operation
            if (num1 != 0 && num2 != 0)
            {
                printf("in operation\n");
                int result = 0;
                switch (op)
                {
                case '+':
                    result = num1 + num2;
                    break;
                case '-':
                    result = num1 - num2;
                    break;
                case '*':
                    result = num1 * num2;
                    break;
                case '/':
                    result = num1 / num2;
                    break;
                }

                // Update cards
                player->nums[selected_index1] = -1;
                player->nums[selected_index2] = result;
                printf("result: %d\n", result);
                player->opStage = SELECT_NUM1; // Reset for next round
                num1 = 0;
                num2 = 0;
                op = ' ';
                // printf("%d\n", player->opStage);
            }

            // Reset buttons
            if (gpio_get(BUTTON_PIN_P1_R) == 0)
            {
                transitionToState(player, GAME_PLAYING);
                break;
            }

            // // If operation all finished, check and transition state
            // int active_cards = 0;
            // int value = -1;
            // for (int i = 0; i < 4; i++)
            // {
            //     if (player->nums[i] != -1)
            //     {
            //         active_cards++;
            //         value = player->nums[i];
            //     }
            // }
            // if (active_cards == 1 && value == 24)
            // {
            //     transitionToState(player, GAME_OVER);
            //     printf("inside finish stage\n");
            //     break;
            // }
        }
        break;
    case GAME_OVER:
        printf("in game over\n");
        if (gpio_get(BUTTON_PIN_P1_E) == 0)
        {
        printf("in game over\n");
        transitionToState(player, START_MENU);
        }
        break;
    }
}


void _generateNumbers(Player *player)
{
    srand(time(NULL));

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
        player->cards[j].y = 0;

        // set destination position of the cards
        int offset = 0;
        if (!player->onLeft)
        {
            offset = 315;
        }
        switch (j)
        {
        case 0:
            player->cards[j].destX = offset + 110;
            player->cards[j].destY = 150;
            break;
        case 2:
            player->cards[j].destX = offset + 5;
            player->cards[j].destY = 250;
            break;
        case 3:
            player->cards[j].destX = offset + 215;
            player->cards[j].destY = 250;
            break;
        case 1:
            player->cards[j].destX = offset + 110;
            player->cards[j].destY = 350;
            break;
        }
    }
}

void slideCards(Player *player)
{

    for (int i = 0; i < 4; i++)
    {

        if (player->cards[i].x != player->cards[i].destX || player->cards[i].y != player->cards[i].destY)
        {
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

            if (player->cards[i].x != player->cards[i].destX || player->cards[i].y != player->cards[i].destY)
            {
                moveImage((const unsigned char *)player->cards[i].image, IMG_HEIGHT, IMG_WIDTH,
                        player->cards[i].x,
                        player->cards[i].y,
                        player->cards[i].x + stepX,
                        player->cards[i].y + stepY);
                player->cards[i].x += stepX;
                player->cards[i].y += stepY;
            }
        }
    }
}

void transitionToState(Player *player, GameState newState)
{

    switch (newState)
    {
    case START_MENU:
        // Clear the screen
        fillRect(0, 0, 640, 480, BLACK);
        pasteImage((const unsigned char *)backOfCard, IMG_HEIGHT, IMG_WIDTH, 20, 20);
        printf("In Start Menu State\n\r");
        player1.currentState = START_MENU;
        player2.currentState = START_MENU;
        break;
    case GAME_PLAYING:
        printf("In Game Playing State\n\r");
        fillRect(0, 0, 640, 480, BLACK);
        _generateNumbers(&player1);
        _generateNumbers(&player2);
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
}

static float progress = 0.0;
void executeStep(Player *player)
{
    // Execute the current game state logic for the given player
    switch (player->currentState)
    {
    case START_MENU:
        // tolerance for floating point comparison
        if (progress <= 1. + 0.01)
        {
            flipImage((const unsigned char *)backOfCard, IMG_HEIGHT, IMG_WIDTH, 20, 20, (const unsigned char *)aceOfClub, progress);
            progress += 0.05;
        }

        // Blink the text
        if ((time_us_32() / 1000000) % 2 == 0)
        {
            fillRect(248, 200, 400, 50, BLACK);
        }
        else
        {
            setCursor(248, 200);
            setTextColor2(WHITE, BLACK);
            writeStringBig("Press any key to start");
        }
        break;
    case GAME_PLAYING:

        // Update the numbers on the screen for this player
        slideCards(player);
        // Check if we have a solution
        // int contains24 = 0;
        // int allNumbersUsed = 1;
        // for (int i = 0; i < 4; i++)
        // {
        //     if (player->nums[i] == 24)
        //     {
        //         contains24 = 1;
        //         break;
        //     }
        //     else if (player->nums[i] != -1)
        //     {
        //         allNumbersUsed = 0;
        //     }
        // }

        // if (contains24 && allNumbersUsed)
        // {
        //     transitionToState(player, GAME_OVER);
        // }
        // else if (allNumbersUsed)
        // {
        //     transitionToState(player, GAME_OVER);
        // }
        break;
    case GAME_OVER:
        // Logic for game over
        break;
    default:
        break;
    }
}
