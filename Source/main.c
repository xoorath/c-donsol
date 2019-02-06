#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <math.h>

#include "donsol-card.h"
#include "donsol-game.h"
#include "cdonsol-art.h"

enum {
    C_DEFAULT = 1,
    C_WHITE_ON_BLACK = 1,
    C_RED_ON_BLACK = 2,
    
    C_BLACK_ON_WHITE = 3,
    C_RED_ON_WHITE = 4,

    C_WHITE_ON_RED = 5,
    C_BLACK_ON_RED = 6
};


//////////////////////////////////////////////////////////////////////////////// Data
static DonsolGame_t g_Game;

static struct Scene_t {
    u8 Width, Height;
    
    u8 CardsX, CardsY, CardSpacing;
    
    char StatusText[64];
    u8 StatusPosX, StatusPosY;

    char const* InputHelpText;
    u8 InputPosX, InputPosY;

    u8 ProgressChars;
    char *ProgressCharFull, *ProgressCharEmpty;

    u8 PotionJustWasted;
    u8 HPPosX, HPPosY;
    u8 DPPosX, DPPosY;
    u8 XPPosX, XPPosY;
} g_Scene = {
    // How big is the play area
    .Width = 120, .Height = 26,

    // Where to draw cards, how far appart
    .CardsX = 13, .CardsY = 9, .CardSpacing = 27,
    
    .StatusText = {0},
    .StatusPosX = 5, .StatusPosY = 3,

    .InputHelpText = "cards: 1 2 3 4   run: space   restart: x   quit: q",

    // Where to take player input; how much input to take
    .InputPosX = 37, 
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

char ErrorBuffer[1024] = {0};
WINDOW *Window = 0;

//////////////////////////////////////////////////////////////////////////////// API

static void Update(void);
static void Render(void);
static void StartGame(void);
static void Exit(void);

//////////////////////////////////////////////////////////////////////////////// Helpers

int GetProgressBar(int value, int maxValue, int outputRange) {
    double v = (double)value / (double)maxValue;
    int prog = (int)floor((v * (double)outputRange + 0.5));
    if(prog < 0) prog = 0;
    else if(prog > outputRange) prog = outputRange;
    return prog;
}

void OnGameError(char const* msg) {
    memcpy(ErrorBuffer, msg, strlen(msg)+1);
}

void OnStatusUpdate(DonsolStatusUpdate_e update, char const* msg) {
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
        case ' ':
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
    sprintf(progressBuffer, "HP %02d [", 
        g_Game.hp);
    wmove(Window, g_Scene.HPPosY, g_Scene.Width - g_Scene.HPPosX);
    wprintw(Window, "%s", progressBuffer);
    sprintf(progressBuffer, "%.*s%.*s",
        progress, 
        g_Scene.ProgressCharFull,
        g_Scene.ProgressChars-progress,
        g_Scene.ProgressCharEmpty);
    attron(COLOR_PAIR(C_RED_ON_BLACK));
    wprintw(Window, "%s", progressBuffer);
    attroff(COLOR_PAIR(C_RED_ON_BLACK));
    wprintw(Window, "]", progressBuffer);

    if(g_Game.hpDelta != 0 && !g_Scene.PotionJustWasted) {
        wmove(Window, g_Scene.HPPosY-1, g_Scene.Width - g_Scene.HPPosX+3);
        wprintw(Window, "%c%d", g_Game.hpDelta>0?'+':'-', abs(g_Game.hpDelta));
    } else if (g_Scene.PotionJustWasted) {
        wmove(Window, g_Scene.HPPosY-1, g_Scene.Width - g_Scene.HPPosX+3);
        wprintw(Window, "+0");

        wmove(Window, g_Scene.HPPosY+1, g_Scene.Width - g_Scene.HPPosX+7);
        wprintw(Window, "wasted");
    }

    // DP
    //////////
    progress = GetProgressBar(g_Game.dp, 11, g_Scene.ProgressChars);
    sprintf(progressBuffer, "DP %02d [%.*s%.*s]", 
        g_Game.dp,
        progress, 
        g_Scene.ProgressCharFull,
        g_Scene.ProgressChars-progress,
        g_Scene.ProgressCharEmpty);
    
    wmove(Window, g_Scene.DPPosY, g_Scene.Width - g_Scene.DPPosX);
    wprintw(Window, "%s", progressBuffer);

    if(g_Game.dpDelta != 0) {
        wmove(Window, g_Scene.DPPosY-1, g_Scene.Width - g_Scene.DPPosX+3);
        wprintw(Window, "%c%d", g_Game.dpDelta>0?'+':'-', abs(g_Game.dpDelta));
    }

    if(g_Game.dp > 0 && g_Game.shieldBreakLimit < 0) {
        wmove(Window, g_Scene.DPPosY+1, g_Scene.Width - g_Scene.DPPosX + 7);
        wprintw(Window, "fresh");
    } else if(g_Game.dp > 0 && g_Game.shieldBreakLimit > 0) {
        wmove(Window, g_Scene.DPPosY+1, g_Scene.Width - g_Scene.DPPosX + 7);
        wprintw(Window, "worn (%d)", (int)g_Game.shieldBreakLimit);
    }

    // XP
    //////////
    progress = GetProgressBar(g_Game.xp, 54, g_Scene.ProgressChars);
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
        wprintw(Window, "%c%d", g_Game.xpDelta>0?'+':'-', g_Game.xpDelta);
    }

    // Cards
    //////////
    for(u8 i = 0; i < 4; ++i) {
        Card_t card = *g_Game.slots[i].dcard;
        char const* art = NULL;

        u8 suit = donsol_card_GetSuit(card);
        u8 isJoke = donsol_card_IsJoker(card);
        u8 isFlipped = donsol_card_IsFlipped(card);
        
        // Handle top text
        char basicName[3] = {' ', ' ', '\0'}; // reuse for card corner
        if(!isFlipped) {
            char cardNameBuff[16] = {0};

            if(isJoke) {
                sprintf(cardNameBuff, "Joker");
            } else {
                if(donsol_card_IsNumeric(card)) {
                    sprintf(basicName, "%d", (int)donsol_card_GetNumericValue(card)+1);
                }
                else if((card & CARD_K) == CARD_K) { sprintf(basicName, "K"); }
                else if((card & CARD_Q) == CARD_Q) { sprintf(basicName, "Q"); }
                else if((card & CARD_J) == CARD_J) { sprintf(basicName, "J"); }
                else if((card & CARD_A) == CARD_A) { sprintf(basicName, "A"); }
                
                if(suit == SUIT_CLUBS)         { sprintf(cardNameBuff, "%s of clubs", basicName); }
                else if(suit == SUIT_DIAMONDS) { sprintf(cardNameBuff, "%s of diamonds", basicName); }
                else if(suit == SUIT_SPADES)   { sprintf(cardNameBuff, "%s of spades", basicName); }
                else if(suit == SUIT_HEARTS)   { sprintf(cardNameBuff, "%s of hearts", basicName); }
            }
            u8 largestNameText = 14;
            u8 nameTextLen = strlen(cardNameBuff);
            u8 centerOffset = (largestNameText-(largestNameText-nameTextLen))/2;
            wmove(Window, g_Scene.CardsY-2, g_Scene.CardsX + (i*g_Scene.CardSpacing) + (cdonsol_art_width/2) - centerOffset);
            wprintw(Window, "%s", cardNameBuff);
        }

        // Handle card art
        int artColor = COLOR_PAIR(C_DEFAULT);
        if(isFlipped) {
            artColor = COLOR_PAIR(C_BLACK_ON_RED) | A_BOLD;
            art = cdonsol_art_back;
        } else if(isJoke) {
            artColor = COLOR_PAIR(C_WHITE_ON_RED) | A_BOLD;
            art = cdonsol_art_joker;
        } else if(suit == SUIT_HEARTS) {
            artColor = COLOR_PAIR(C_RED_ON_WHITE) | A_BOLD;
            if(donsol_card_IsNumeric(card)) {
                art = cdonsol_art_heart;
            } else {
                art = cdonsol_art_white_mage;
            }
        } else if(suit == SUIT_DIAMONDS) {
            artColor = COLOR_PAIR(C_RED_ON_WHITE) | A_BOLD;
            if(donsol_card_IsNumeric(card)) {
                art = cdonsol_art_diamond;
            } else {
                art = cdonsol_art_red_mage;
            }
        } else if(suit == SUIT_CLUBS) {
            artColor = COLOR_PAIR(C_BLACK_ON_WHITE) | A_BOLD;
            art = cdonsol_art_club;
        } else if(suit == SUIT_SPADES) {
            artColor = COLOR_PAIR(C_BLACK_ON_WHITE) | A_BOLD;
            art = cdonsol_art_spade;
        }
        attron(artColor);
        for(u16 y = 0; y < cdonsol_art_height; ++y) {
            for(u16 x = 0; x < cdonsol_art_width; ++x) {
                wmove(Window, g_Scene.CardsY+y, g_Scene.CardsX+x + (i*g_Scene.CardSpacing));
                waddch(Window, art[x+y*cdonsol_art_width]);
            }
        }
        if(!isFlipped && !isJoke) {
            if(basicName[1]) {
                wmove(Window, g_Scene.CardsY, g_Scene.CardsX + cdonsol_art_width-3 + (i*g_Scene.CardSpacing));
                waddch(Window, basicName[0]);
                waddch(Window, basicName[1]);
            } else if(basicName[0]) {
                wmove(Window, g_Scene.CardsY, g_Scene.CardsX + cdonsol_art_width-2 + (i*g_Scene.CardSpacing));
                waddch(Window, basicName[0]);
            }
        }

        attroff(artColor);

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
#undef ZERO_ARR
}

//////////////////////////////////////////////////////////////////////////////// Exit
static void Exit(void) {
    WINDOW *dialogue = newwin(g_Scene.Height/2, g_Scene.Width/2, g_Scene.Height/4, g_Scene.Width/4);
    if(0 != dialogue) {
        
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
    }

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

    // Setup colors
    start_color();
    init_pair(C_WHITE_ON_BLACK, COLOR_WHITE, COLOR_BLACK);
    init_pair(C_RED_ON_BLACK, COLOR_RED, COLOR_BLACK);
    init_pair(C_BLACK_ON_WHITE, COLOR_BLACK, COLOR_WHITE);
    init_pair(C_RED_ON_WHITE, COLOR_RED, COLOR_WHITE);
    init_pair(C_WHITE_ON_RED, COLOR_WHITE, COLOR_RED);
    init_pair(C_BLACK_ON_RED, COLOR_BLACK, COLOR_RED);

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