#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <math.h>

#include "donsol-card.h"
#include "donsol-game.h"

#define INPUT_IS(x) (0 == strcmp(InputBuffer, x))

//////////////////////////////////////////////////////////////////////////////// Data
static DonsolGame_t g_Game;

static struct Scene_t {
    int Width, Height;
    
    int CardsX, CardsY, CardSpacing;
    
    char const* WelcomeText;
    int WelcomePosX, WelcomePosY;

    char const* InputPrompt; int InputPromptLen;
    char const* InputHelpText;
    int InputPosX, InputPosY, InputWidth;

    int ProgressChars;
    char *ProgressCharFull, *ProgressCharEmpty;

    int HPPosX, HPPosY;
    int DPPosX, DPPosY;
    int XPPosX, XPPosY;
} g_Scene = {
    // How big is the play area
    .Width = 120, .Height = 30,

    // Where to draw cards, how far appart
    .CardsX = 5, .CardsY = 5, .CardSpacing = 10,
    
    .WelcomeText = "Entered Donsol",
    .WelcomePosX = 5, .WelcomePosY = 3,

    .InputPrompt = "Input: ", .InputPromptLen = /*compute me*/0,

    .InputHelpText = "cards: [1 2 3 4] run [r] restart [x] quit [q]",

    // Where to take player input; how much input to take
    .InputPosX = 45, 
    .InputPosY = 5, // from screen bottom
    .InputWidth = 64,
    
    // How many characters are represented in the progress bars
    .ProgressChars = 8,
    .ProgressCharFull = "########",
    .ProgressCharEmpty = "--------",

    .HPPosX=60,
    .HPPosY=3,
    
    .DPPosX=40,
    .DPPosY=3,

    .XPPosX=20,
    .XPPosY=3,
};

char InputBuffer[1024] = {0};
WINDOW *Window;

//////////////////////////////////////////////////////////////////////////////// API

static void Update(void);
static void Render(void);
static void StartGame(void);
static void Exit(void);

//////////////////////////////////////////////////////////////////////////////// Helpers

int GetProgressBar(int value, int maxValue, int outputRange) {
    double v = (double)value / (double)maxValue;
    return (int)floor((v * (double)outputRange + 0.5));
}

//////////////////////////////////////////////////////////////////////////////// Update
static void Update(void) {
    // Read Input
    //////////
    wmove(Window, g_Scene.Height - g_Scene.InputPosY, g_Scene.InputPosX);
    wscanw(Window, "%s", InputBuffer);

    // Check Input
    //////////
    if(INPUT_IS("q")) {
        Exit();
    }
    else if(INPUT_IS("x")) {
        StartGame();
    }
    else if(INPUT_IS("r")) {
        donsol_game_run(&g_Game);
    }
    else if(INPUT_IS("1")) {
        donsol_game_pick_card(&g_Game, 1);
    }
    else if(INPUT_IS("2")) {
        donsol_game_pick_card(&g_Game, 2);
    }
    else if(INPUT_IS("3")) {
        donsol_game_pick_card(&g_Game, 3);
    }
    else if(INPUT_IS("4")) {
        donsol_game_pick_card(&g_Game, 4);
    }

}

//////////////////////////////////////////////////////////////////////////////// Render
static void Render(void) {
    // Clear/Draw Window
    //////////
    wclear(Window);
    box(Window, 0, 0);

    // Welcome Text
    //////////
    wmove(Window, g_Scene.WelcomePosY, g_Scene.WelcomePosX);
    wprintw(Window, "%s", g_Scene.WelcomeText);

    // HP
    //////////
    char progressBuffer[32] = {0};
    int progress = GetProgressBar(g_Game.hp, 21, g_Scene.ProgressChars);
    sprintf(progressBuffer, "HP %02d [%.*s%.*s]", 
        g_Game.hp, 
        progress, 
        g_Scene.ProgressCharFull,
        g_Scene.ProgressChars-progress,
        g_Scene.ProgressCharEmpty);
    
    wmove(Window, g_Scene.HPPosY, g_Scene.Width - g_Scene.HPPosX);
    wprintw(Window, "%s", progressBuffer);

    // DP
    //////////
    progress = GetProgressBar(g_Game.dp, 21, g_Scene.ProgressChars);
    sprintf(progressBuffer, "DP %02d [%.*s%.*s]", 
        g_Game.dp,
        progress, 
        g_Scene.ProgressCharFull,
        g_Scene.ProgressChars-progress,
        g_Scene.ProgressCharEmpty);
    
    wmove(Window, g_Scene.DPPosY, g_Scene.Width - g_Scene.DPPosX);
    wprintw(Window, "%s", progressBuffer);

    // XP
    //////////
    progress = GetProgressBar(g_Game.xp, 21, g_Scene.ProgressChars);
    sprintf(progressBuffer, "XP %02d [%.*s%.*s]", 
        g_Game.xp,
        progress, 
        g_Scene.ProgressCharFull,
        g_Scene.ProgressChars-progress,
        g_Scene.ProgressCharEmpty);
    
    wmove(Window, g_Scene.XPPosY, g_Scene.Width - g_Scene.XPPosX);
    wprintw(Window, "%s", progressBuffer);

    // Help Text
    //////////
    wmove(Window, g_Scene.Height - g_Scene.InputPosY+1, g_Scene.InputPosX - g_Scene.InputPromptLen);
    wprintw(Window, "%s", g_Scene.InputHelpText);
    
    // Input Prompt
    //////////
    wmove(Window, g_Scene.Height - g_Scene.InputPosY, g_Scene.InputPosX - g_Scene.InputPromptLen);
    wprintw(Window, g_Scene.InputPrompt);


    wrefresh(Window);
}

//////////////////////////////////////////////////////////////////////////////// Start Game
static void StartGame(void) {
#define ZERO_ARR(x) memset(x, 0, sizeof(x)/sizeof(*x));

    donsol_game_start(&g_Game);
    ZERO_ARR(InputBuffer);
    g_Scene.InputPromptLen = strlen(g_Scene.InputPrompt);

#undef ZERO_ARR
}

//////////////////////////////////////////////////////////////////////////////// Exit
static void Exit(void) {
    donsol_game_quit(&g_Game);
    delwin(Window);
    endwin();
    refresh();
    exit(0);
}

//////////////////////////////////////////////////////////////////////////////// Main
int main() {
    if ( (Window = initscr()) == NULL ) {
	    fprintf(stderr, "Error initializing ncurses.\n");
	    exit(EXIT_FAILURE);
    }
    wresize(Window, g_Scene.Height, g_Scene.Width);
    StartGame();
    for(;;) {
        Render();
        Update();
    }
}