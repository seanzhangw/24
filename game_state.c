#include "assets.h"

#include "game_state.h"
#include "array_sol.h"

// Initialize the current game state
GameState currentState = START_MENU;
int stateChange = 0;

int nums[4] = {-1, -1, -1, -1}; // Initialize numbers to -1

#define MAX_LINE_LEN 2048
#define MAX_LINES 766

void sol_init() {
    array_solutions(100);
    for (int i = 0; i < 100; ++i) {
        printf("%d %d %d %d\n", arrSol[i][0], arrSol[i][1], arrSol[i][2], arrSol[i][3]);
    }
}

void _generateNumbers()
{
    FILE *file = fopen("collection_with_solution_with_methods.txt", "r");

    // Step 1: Count the total number of lines
    int total_lines = 0;
    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, sizeof(buffer), file))
    {
        total_lines++;
    }

    if (total_lines == 0)
    {
        printf("No lines found.\n");
        fclose(file);
    }

    // Step 2: Pick a random line index
    srand(time(NULL));
    int random_line_index = rand() % total_lines;

    // Step 3: Read the file again and stop at the random line
    rewind(file); // Go back to the beginning
    int current_line = 0;
    while (fgets(buffer, sizeof(buffer), file))
    {
        if (current_line == random_line_index)
        {
            break;
        }
        current_line++;
    }
    fclose(file);

    // Step 4: Parse the line
    int a, b, c, d, target;
    if (sscanf(buffer, "%d %d %d %d: [%d]", &a, &b, &c, &d, &target) == 5)
    {
        printf("Selected Line: %s\n", buffer);
        printf("Numbers: %d %d %d %d\n", a, b, c, d);
        printf("Target: %d\n", target);
    }
    else
    {
        printf("Failed to parse line: %s\n", buffer);
    }
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

        pasteImage(&tenOfHeart[0][0], IMG_HEIGHT, IMG_WIDTH, 10, 10);
        //pasteImage(&background[0][0], 639, 479, 10, 10);

        // Logic for start

        // Blink the text
        if ((time_us_32() / 1000000) % 2 == 0)
        {
            fillRect(248, 200, 400, 50, BLACK);
        }
        else
        {
            // 340 is the cetner of the screen. "Press any key to start" is 23 characters long.
            // 23 * 8 = 184. 184 /2 = 92. 340 - 92 = 248.

            setCursor(248, 200);
            setTextColor2(WHITE, BLACK);
            writeStringBig("Press any key to start");
        }
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