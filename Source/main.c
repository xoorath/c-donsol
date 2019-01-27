#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <math.h>

#include "donsol-card.h"
#include "donsol-game.h"
#include "cdonsol-art.h"

#define INPUT_IS(x) (0 == strcmp(InputBuffer, x))

//////////////////////////////////////////////////////////////////////////////// Data
static DonsolGame_t g_Game;

static struct Scene_t {
    int Width, Height;
    
    int CardsX, CardsY, CardSpacing;
    
    char StatusText[64];
    int StatusPosX, StatusPosY;

    char const* InputHelpText;
    int InputPosX, InputPosY;

    int ProgressChars;
    char *ProgressCharFull, *ProgressCharEmpty;

    u8 PotionJustWasted;
    int HPPosX, HPPosY;
    int DPPosX, DPPosY;
    int XPPosX, XPPosY;
} g_Scene = {
    // How big is the play area
    .Width = 120, .Height = 26,

    // Where to draw cards, how far appart
    .CardsX = 12, .CardsY = 10, .CardSpacing = 30,
    
    .StatusText = {0},
    .StatusPosX = 5, .StatusPosY = 3,

    .InputHelpText = "cards: 1 2 3 4   run: r   restart: x   quit: q",

    // Where to take player input; how much input to take
    .InputPosX = 38, 
    .InputPosY = 5, // from screen bottom
    
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
char ErrorBuffer[1024] = {0};
WINDOW *Window = 0, *dialogue = 0;

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

void OnGameError(char const* msg) {
    memcpy(ErrorBuffer, msg, strlen(msg)+1);
}

void OnStatusUpdate(DonsolStatusUpdate update, char const* msg) {
    sprintf(g_Scene.StatusText, "%s", msg);

    g_Scene.PotionJustWasted = (update == DONSOL_STATUS_POTION_WASTED);
}

//////////////////////////////////////////////////////////////////////////////// Update
static void Update(void) {
    // Read Input
    //////////
    int c = wgetch(Window);

    // Check Input
    //////////
    switch(c) {
        case 27: // esc
        case 'q':
            Exit();
            break;
        case 'x':
            StartGame();
            break;
        case 'r':
            donsol_game_pick_run(&g_Game);
            break;
        case '1':
            donsol_game_pick_card(&g_Game, 1);
            break;
        case '2':
            donsol_game_pick_card(&g_Game, 2);
            break;
        case '3':
            donsol_game_pick_card(&g_Game, 3);
            break;
        case '4':
            donsol_game_pick_card(&g_Game, 4);
            break;
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
    wmove(Window, g_Scene.StatusPosY, g_Scene.StatusPosX);
    wprintw(Window, "%s", g_Scene.StatusText);

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

    if(g_Game.hpDelta > 0 && !g_Scene.PotionJustWasted) {
        wmove(Window, g_Scene.HPPosY-1, g_Scene.Width - g_Scene.HPPosX+3);
        wprintw(Window, "+%d", g_Game.hpDelta);
    } else if (g_Scene.PotionJustWasted) {
        wmove(Window, g_Scene.HPPosY-1, g_Scene.Width - g_Scene.HPPosX);
        wprintw(Window, "wasted");
    }

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

    if(g_Game.dpDelta > 0) {
        wmove(Window, g_Scene.DPPosY-1, g_Scene.Width - g_Scene.DPPosX+3);
        wprintw(Window, "+%d", g_Game.dpDelta);
    }

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

    if(g_Game.xpDelta > 0) {
        wmove(Window, g_Scene.XPPosY-1, g_Scene.Width - g_Scene.XPPosX+3);
        wprintw(Window, "+%d", g_Game.xpDelta);
    }

    // Cards
    //////////
    for(u8 i = 0; i < 4; ++i) {
        card_t card = *g_Game.slots[i].dcard;
        char const* art = NULL;

        u8 suit = donsol_card_GetSuit(card);
        u8 isJoke = donsol_card_IsJoker(card);
        u8 isFlipped = donsol_card_IsFlipped(card);
        
        // Handle top text
        if(!isFlipped) {
            char cardNameBuff[16] = {0};

            if(isJoke) {
                sprintf(cardNameBuff, "Joker");
            } else {
                char basicName[8] = {0};
                if(donsol_card_IsNumeric(card)) {
                    sprintf(basicName, "%d", (int)donsol_card_GetNumericValue(card)+1);
                } else if(card & CARD_K) {
                    sprintf(basicName, "K");
                } else if(card & CARD_Q) {
                    sprintf(basicName, "Q");
                } else if(card & CARD_J) {
                    sprintf(basicName, "J");
                } else {
                    sprintf(basicName, "A");
                }

                if(suit == SUIT_CLUBS) {
                    sprintf(cardNameBuff, "%s of clubs", basicName);
                } else if(suit == SUIT_DIAMONDS) {
                    sprintf(cardNameBuff, "%s of diamonds", basicName);
                } else if(suit == SUIT_SPADES) {
                    sprintf(cardNameBuff, "%s of spades", basicName);
                } else {
                    sprintf(cardNameBuff, "%s of hearts", basicName);
                }
            }
            u8 largestNameText = 14;
            u8 nameTextLen = strlen(cardNameBuff);
            u8 centerOffset = (largestNameText-(largestNameText-nameTextLen))/2;
            wmove(Window, g_Scene.CardsY-2, g_Scene.CardsX + (i*g_Scene.CardSpacing) + (cdonsol_art_width/2) - centerOffset);
            wprintw(Window, "%s", cardNameBuff);
        }

        // Handle card art
        if(isFlipped) {
            art = cdonsol_art_back;
        } else if(isJoke) {
            art = cdonsol_art_joker;
        } else if(suit == SUIT_HEARTS) {
            art = cdonsol_art_heart;
        } else if(suit == SUIT_DIAMONDS) {
            art = cdonsol_art_diamond;
        } else if(suit == SUIT_CLUBS) {
            art = cdonsol_art_club;
        } else if(suit == SUIT_SPADES) {
            art = cdonsol_art_spade;
        }
        for(u16 y = 0; y < cdonsol_art_height; ++y) {
            for(u16 x = 0; x < cdonsol_art_width; ++x) {
                wmove(Window, g_Scene.CardsY+y, g_Scene.CardsX+x + (i*g_Scene.CardSpacing));
                waddch(Window, art[x+y*cdonsol_art_width]);
            }
        }

        // handle bottom text
        if(!isFlipped) {
            // largest text:
            // "second donsol 21"
            u8 largestNameText = 15;
            u8 nameTextLen = strlen(g_Game.slots[i].name);
            u8 centerOffset = (largestNameText-(largestNameText-nameTextLen))/2;
            wmove(Window, 
                g_Scene.CardsY+cdonsol_art_height+1, 
                g_Scene.CardsX + (i*g_Scene.CardSpacing) + (cdonsol_art_width/2) - centerOffset);
            wprintw(Window, "%s", g_Game.slots[i].name);
        }
    }

    // Help Text
    //////////
    wmove(Window, g_Scene.Height - g_Scene.InputPosY+1, g_Scene.InputPosX);
    wprintw(Window, "%s", g_Scene.InputHelpText);
    
    if(ErrorBuffer[0]) {
        wmove(Window, 0, 0);
        wprintw(Window, ErrorBuffer);
    }

    wrefresh(Window);
}

//////////////////////////////////////////////////////////////////////////////// Start Game
static void StartGame(void) {
#define ZERO_ARR(x) memset(x, 0, sizeof(x)/sizeof(*x));
    donsol_game_start(&g_Game);
    ZERO_ARR(InputBuffer);
#undef ZERO_ARR
}

//////////////////////////////////////////////////////////////////////////////// Exit
static void Exit(void) {
    dialogue = newwin(g_Scene.Height/2, g_Scene.Width/2, g_Scene.Height/4, g_Scene.Width/4);
    if(0 == dialogue) {
        goto exit_label;
    }

    box(dialogue, 0, 0);

    int selected = 0;
    while(!selected) {
        wmove(dialogue, 6, 24);
        wprintw(dialogue, "Quit? (Y/N)");
        wmove(dialogue, 0, 0);
        switch(wgetch(dialogue)) {
            default:
                break;
            case 'y':
            case 'Y':
                selected = 1;
                break;
            case 27: // esc
            case 'n':
            case 'N':
                delwin(dialogue);
                dialogue = 0;
                return;
        }
    }

    delwin(dialogue);
    dialogue = 0;

exit_label:
    donsol_game_quit(&g_Game);
    delwin(Window);
    endwin();
    refresh();
    exit(0);
}

//////////////////////////////////////////////////////////////////////////////// Main
int main() {
    if (NULL == (Window = initscr())) {
	    fprintf(stderr, "Error initializing ncurses.\n");
	    exit(EXIT_FAILURE);
    }
    // size the window
    wresize(Window, g_Scene.Height, g_Scene.Width);

    // hide the cursor
    curs_set(0);
    // don't show the character you just typed.
    noecho();

    // start the game off fresh before we start it 
    // (important for null checking pointers in the game object)
    memset(&g_Game, 0, sizeof(g_Game));

    // hook up error handler
    g_Game.onError = OnGameError;
    g_Game.onStatusUpdate = OnStatusUpdate;

    StartGame();
    for(;;) {
        Render();
        Update();
    }
}