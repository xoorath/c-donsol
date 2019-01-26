#ifndef __DONSOL_GAME_H_
#define __DONSOL_GAME_H_

#include <PR/ultratypes.h>
#include "donsol-card.h"

typedef struct {
    // last change to hp/xp/dp for use in UI
    s8 hpDelta, xpDelta, dpDelta;

    // current hp/xp/dp
    u8 hp, xp, dp;

    // The 4 active slots.
    card_t card[4];
    card_t *deck;
} DonsolGame_t;

// init the game object
void donsol_game_start(DonsolGame_t* game);

void donsol_game_run(DonsolGame_t* game);
void donsol_game_pick_card(DonsolGame_t* game, u8 index); // index can be: [1,2,3,4]

// perform cleanup
void donsol_game_quit(DonsolGame_t* game);
#endif // __DONSOL_GAME_H_