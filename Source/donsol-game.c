#include "donsol-game.h"
#include <stdio.h>


const u8 g_DeckSize = 54;
static card_t g_Deck[54] = {
    CARD_JO1,
    CARD_JO2,

    CARD_A    | SUIT_HEARTS,
    CARD_2    | SUIT_HEARTS,
    CARD_3    | SUIT_HEARTS,
    CARD_4    | SUIT_HEARTS,
    CARD_5    | SUIT_HEARTS,
    CARD_6    | SUIT_HEARTS,
    CARD_7    | SUIT_HEARTS,
    CARD_8    | SUIT_HEARTS,
    CARD_9    | SUIT_HEARTS,
    CARD_10   | SUIT_HEARTS,
    CARD_J    | SUIT_HEARTS,
    CARD_Q    | SUIT_HEARTS,
    CARD_K    | SUIT_HEARTS,

    CARD_A    | SUIT_DIAMONDS,
    CARD_2    | SUIT_DIAMONDS,
    CARD_3    | SUIT_DIAMONDS,
    CARD_4    | SUIT_DIAMONDS,
    CARD_5    | SUIT_DIAMONDS,
    CARD_6    | SUIT_DIAMONDS,
    CARD_7    | SUIT_DIAMONDS,
    CARD_8    | SUIT_DIAMONDS,
    CARD_9    | SUIT_DIAMONDS,
    CARD_10   | SUIT_DIAMONDS,
    CARD_J    | SUIT_DIAMONDS,
    CARD_Q    | SUIT_DIAMONDS,
    CARD_K    | SUIT_DIAMONDS,

    CARD_A    | SUIT_SPADES,
    CARD_2    | SUIT_SPADES,
    CARD_3    | SUIT_SPADES,
    CARD_4    | SUIT_SPADES,
    CARD_5    | SUIT_SPADES,
    CARD_6    | SUIT_SPADES,
    CARD_7    | SUIT_SPADES,
    CARD_8    | SUIT_SPADES,
    CARD_9    | SUIT_SPADES,
    CARD_10   | SUIT_SPADES,
    CARD_J    | SUIT_SPADES,
    CARD_Q    | SUIT_SPADES,
    CARD_K    | SUIT_SPADES,

    CARD_A    | SUIT_CLUBS,
    CARD_2    | SUIT_CLUBS,
    CARD_3    | SUIT_CLUBS,
    CARD_4    | SUIT_CLUBS,
    CARD_5    | SUIT_CLUBS,
    CARD_6    | SUIT_CLUBS,
    CARD_7    | SUIT_CLUBS,
    CARD_8    | SUIT_CLUBS,
    CARD_9    | SUIT_CLUBS,
    CARD_10   | SUIT_CLUBS,
    CARD_J    | SUIT_CLUBS,
    CARD_Q    | SUIT_CLUBS,
    CARD_K    | SUIT_CLUBS
};

static void donsol_game_set_slot(DonsolGame_t* game, u8 slotIndex, u8 deckIndex) {
    if(slotIndex >= 4) {
        if(game->onError) game->onError("set slot slot index out of range");
        return;
    }
    if(deckIndex >= g_DeckSize) {
        if(game->onError) game->onError("set slot deck index out of range");
        return;
    }
    
    card_t* dcard = &g_Deck[deckIndex];
    DonsolCardDescription_t* desc = &game->slots[slotIndex];    

    desc->dcard = dcard;
    if(donsol_card_IsJoker(*dcard)) {
        desc->isMonster = 1;
        desc-> isPotion = 0;
        desc->isShield = 0;
        desc->power = 21;
        if(*dcard & CARD_JO1) {
            sprintf(desc->name, "First Donsol 21");
        } else {
            sprintf(desc->name, "Second Donsol 21");
        }
    }
    else if(donsol_card_IsHearts(*dcard)) {
        if(donsol_card_IsNumeric(*dcard)) {
            // potion
            desc->power = donsol_card_GetNumericValue(*dcard) + 1;
            desc->isMonster = 0;
            desc-> isPotion = 1;
            desc->isShield = 0;
            if(desc->power <= 3) {
                sprintf(desc->name, "Sm Potion %d", (int)desc->power);
            } else if(desc->power <= 8) {
                sprintf(desc->name, "Md Potion %d", (int)desc->power);
            } else {
                sprintf(desc->name, "Lg Potion %d", (int)desc->power);
            }
            if(desc->power == 0) {
                char tempbitching[256] = {0};
                sprintf(tempbitching, "%d", (int)*dcard);
                game->onError(tempbitching);
            }
        } else {
            // white mage
            desc->power = 11;
            desc->isMonster = 0;
            desc-> isPotion = 1;
            desc->isShield = 0;
            sprintf(desc->name, "White Mage %d", (int)desc->power);
        }
    }
    else if(donsol_card_IsDiamonds(*dcard)) {
        if(donsol_card_IsNumeric(*dcard)) {
            desc->power = donsol_card_GetNumericValue(*dcard) + 1;
            desc->isMonster = 0;
            desc-> isPotion = 0;
            desc->isShield = 1;
            if(desc->power <= 3) {
                sprintf(desc->name, "Buckler %d", (int)desc->power);
            } else if(desc->power <= 8) {
                sprintf(desc->name, "Shield %d", (int)desc->power);
            } else {
                sprintf(desc->name, "Lg Shield %d", (int)desc->power);
            }
        } else {
            desc->power = 11;
            desc->isMonster = 0;
            desc-> isPotion = 1;
            desc->isShield = 0;
            sprintf(desc->name, "Red Mage %d", (int)desc->power);
        }
    }
    else if(donsol_card_IsClubs(*dcard) || donsol_card_IsSpades(*dcard)) {
        u8 numeric = donsol_card_GetNumericValue(*dcard) + 1;
        if(donsol_card_IsNumeric(*dcard)) {
            desc->power = numeric;
            desc->isMonster = 1;
            desc-> isPotion = 0;
            desc->isShield = 0;

            static char const* clubNames[] = {
                "??", // A
                "Rat", // 2
                "Bat", // 3
                "Imp", // 4
                "Goblin", // 5
                "Orc", // 6
                "Ogre", // 7
                "Beholder", // 8
                "Medusa", // 9
                "Demon"}; // 10
            static char const* spadeNames[] = {
                "??", // A
                "Slime", // 2
                "Tunneler", // 3
                "Fiend", // 4
                "Drake", // 5
                "Specter", // 6
                "Ghost", // 7
                "Elemental", // 8
                "Witch", // 9
                "Familiar"}; // 10
            sprintf(desc->name, "%s %d", donsol_card_IsClubs(*dcard) ? clubNames[desc->power-1] : spadeNames[desc->power-1], (int)desc->power);
        } else {
            desc->isMonster = 1;
            desc-> isPotion = 0;
            desc->isShield = 0;
            switch(numeric) {
                case CARD_J:
                    desc->power = 11;
                    sprintf(desc->name, "Consort %d", (int)desc->power);
                    break;
                case CARD_Q:
                    desc->power = 13;
                    sprintf(desc->name, "Queen %d", (int)desc->power);
                    break;
                case CARD_K:
                    desc->power = 15;
                    sprintf(desc->name, "Regnant %d", (int)desc->power);
                    break;
                case CARD_A:
                    desc->power = 17;
                    sprintf(desc->name, "Empress %d", (int)desc->power);
                    break;
            }
        }
    } else {
        if(game->onError) game->onError("Couldn't determine which card case to handle");
    }
}

static void donsol_game_clear_deltas(DonsolGame_t* game) {
    game->xpDelta = game->hpDelta = game->dpDelta = 0;
}

static void donsol_game_pick_potion(DonsolGame_t* game, card_t *dcard, u8 val) {
    donsol_game_clear_deltas(game);
    if(!game->canDrink) {
        game->onStatusUpdate(DONSOL_STATUS_POTION_WASTED, "Potion Wasted");
        return;
    }

    u8 hp = game->hp;
    game->hp = game->hp+val >= 21 ? 21 : game->hp+val;
    game->hpDelta = game->hp - hp;
    if(game->hpDelta == 0) {
        game->onStatusUpdate(DONSOL_STATUS_POTION_WASTED, "Potion Wasted");
    } else {
        game->onStatusUpdate(DONSOL_STATUS_POTION_USED, "Potion Used");
    }

    game->canDrink = 0;
    game->canRun = 1;
}

static void donsol_game_pick_shield(DonsolGame_t* game, card_t *dcard, int power) {
    game->canDrink = 1;
}

static void donsol_game_pick_enemy(DonsolGame_t* game, card_t *dcard, int atk) {
    

    game->canDrink = 1;
    game->canRun = 1;
}

//////////////////////////////////////////////////////////////////////////////////////////

void donsol_game_pick_run(DonsolGame_t* game) {
    if(!game->canRun) {
        game->onStatusUpdate(DONSOL_STATUS_CANT_RUN, "You can't escape this room.");
        return;
    }

    game->canDrink = 1;
    game->canRun = 0;
}

void donsol_game_start(DonsolGame_t* game) {
    donsol_card_ClearFlippedBit(g_Deck, g_DeckSize);
    donsol_card_ShuffleDeck(g_Deck, g_DeckSize);

    game->hp = 5;
    game->dp = 0;
    game->xp = 0;

    donsol_game_clear_deltas(game);

    game->canDrink = 1;
    donsol_game_set_slot(game, 0, 0);
    donsol_game_set_slot(game, 1, 1);
    donsol_game_set_slot(game, 2, 2);
    donsol_game_set_slot(game, 3, 3);

    game->onStatusUpdate(DONSOL_STATUS_WELCOME, "Entered Donsol.");
}

void donsol_game_pick_card(DonsolGame_t* game, u8 index) {

    if(index == 0 || index > 4) {
        if(game->onError) game->onError("pick card out of range.");
        return;
    }
    DonsolCardDescription_t* desc = &game->slots[index-1];
    card_t *dcard = desc->dcard;
    
    // Couldn't find that card in the deck. This is an error case.
    if(0 == dcard) {
        if(game->onError) game->onError("Selected card has a null dcard");
        return;
    }

    if(desc->isMonster) {
        // donsol
        donsol_game_pick_enemy(game, dcard, desc->power);
    }
    else if(desc->isShield) {
        donsol_game_pick_shield(game, dcard, desc->power);
    }
    else if(desc->isPotion) {
        donsol_game_pick_potion(game, dcard, desc->power);
    } else {
        if(game->onError) game->onError("Couldn't determine which card case to handle");
    }

    *dcard |= CARDSTATE_FLIPPED;
    

}

void donsol_game_quit(DonsolGame_t* game) {

}