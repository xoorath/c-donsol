#include "donsol-game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const u8 g_DeckSize = 54;
static Card_t g_Deck[54] = {
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
    
    Card_t* dcard = &g_Deck[deckIndex];
    DonsolCardDescription_t* desc = &game->slots[slotIndex];    
    sprintf(desc->simpleName, "unknown");
    sprintf(desc->name, "unknown : ??");
    desc->isMonster = 1;
    desc->isPotion = 0;
    desc->isShield = 0;
    desc->power = 1;
    desc->dcard = dcard;

    if(desc->dcard == 0) {
        char errBuff[64] = {0};
        sprintf(errBuff, "Not setting a card [%d %d]", (int)slotIndex, (int)deckIndex);
        if(game->onError) game->onError(errBuff);
        return;
    }

    if(donsol_card_IsJoker(*dcard)) {
        desc->isMonster = 1;
        desc-> isPotion = 0;
        desc->isShield = 0;
        desc->power = 21;
        if(*dcard & CARD_JO1) {
            sprintf(desc->simpleName, "First Donsol");
        } else {
            sprintf(desc->simpleName, "Second Donsol");
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
                sprintf(desc->simpleName, "Sm Potion");
            } else if(desc->power <= 8) {
                sprintf(desc->simpleName, "Md Potion");
            } else {
                sprintf(desc->simpleName, "Lg Potion");
            }
        } else {
            // white mage
            desc->power = 11;
            desc->isMonster = 0;
            desc-> isPotion = 1;
            desc->isShield = 0;
            
            sprintf(desc->simpleName, "White Mage");
        }
    }
    else if(donsol_card_IsDiamonds(*dcard)) {
        if(donsol_card_IsNumeric(*dcard)) {
            desc->power = donsol_card_GetNumericValue(*dcard) + 1;
            desc->isMonster = 0;
            desc-> isPotion = 0;
            desc->isShield = 1;
            if(desc->power <= 3) {
                sprintf(desc->simpleName, "Buckler");
            } else if(desc->power <= 8) {
                sprintf(desc->simpleName, "Shield");
            } else {
                sprintf(desc->simpleName, "Lg Shield");
            }
        } else {
            desc->power = 11;
            desc->isMonster = 0;
            desc-> isPotion = 0;
            desc->isShield = 1;
            sprintf(desc->simpleName, "Red Mage");
        }
    }
    else if(donsol_card_IsClubs(*dcard) || donsol_card_IsSpades(*dcard)) {
        u8 numeric = donsol_card_GetNumericValue(*dcard);
        if(donsol_card_IsNumeric(*dcard)) {
            desc->power = numeric + 1;
            desc->isMonster = 1;
            desc-> isPotion = 0;
            desc->isShield = 0;

            static char const* clubNames[] = { "??", "Rat", "Bat", "Imp", "Goblin", "Orc", "Ogre", "Beholder", "Medusa", "Demon"};
            static char const* spadeNames[] = { "??", "Slime", "Tunneler", "Fiend", "Drake", "Specter", "Ghost", "Elemental", "Witch", "Familiar"};
            sprintf(desc->simpleName, "%s", donsol_card_IsClubs(*dcard) ? clubNames[desc->power-1] : spadeNames[desc->power-1]);
        } else {
            desc->isMonster = 1;
            desc-> isPotion = 0;
            desc->isShield = 0;
            switch(numeric) {
                case CARD_J:
                    desc->power = 11;
                    sprintf(desc->simpleName, "Consort");
                    break;
                case CARD_Q:
                    desc->power = 13;
                    sprintf(desc->simpleName, "Queen");
                    break;
                case CARD_K:
                    desc->power = 15;
                    sprintf(desc->simpleName, "Regnant");
                    break;
                case CARD_A:
                    desc->power = 17;
                    sprintf(desc->simpleName, "Empress");
                    break;
                default:
                    {
                        char errBuff[64] = {0};
                        sprintf(errBuff, "Couldnt figure out face card type. numeric: %d dcard: %d", (int)numeric, (int)*dcard);
                        if(game->onError) game->onError(errBuff);
                    }
                    return;
            }
        }
    } else {
        if(game->onError) game->onError("Couldn't determine which card case to handle");
    }

    sprintf(desc->name, "%s : %d", desc->simpleName, (int)desc->power);
}

static Card_t* donsol_game_next_unflipped_card(DonsolGame_t* game, Card_t* startingCard) {
    if(0 == startingCard) {
        return 0;
    }

    for(Card_t* i = startingCard+1; i < &g_Deck[g_DeckSize]; ++i) {
        if(!donsol_card_IsFlipped(*i)) {
            return i;
        }
    }
    for(Card_t* i = &g_Deck[0]; i < startingCard; ++i) {
        if(!donsol_card_IsFlipped(*i)) {
            return i;
        }
    }
    return 0;
}

static void donsol_game_deal4(DonsolGame_t* game) {
    Card_t* newSlots[4] = {0};
    newSlots[0] = donsol_game_next_unflipped_card(game, game->slots[3].dcard);
    newSlots[1] = donsol_game_next_unflipped_card(game, newSlots[0]);
    newSlots[2] = donsol_game_next_unflipped_card(game, newSlots[1]);
    newSlots[3] = donsol_game_next_unflipped_card(game, newSlots[2]);

    // eliminate duplicates (happens if we wrap around the deck)
    for(u8 i = 0; i < 4; ++i) {
        for(u8 j = 0; j < 4; ++j) {
            if(i != j) {
                if(newSlots[i] == newSlots[j]) {
                    newSlots[j] = 0;
                }
            }
        }
    }

    for(u8 i = 0; i < 4; ++i) {
        if(newSlots[i] != 0) {
            for(u8 j = 0; j < g_DeckSize; ++j) {
                if(&g_Deck[j] == newSlots[i]) {
                    donsol_game_set_slot(game, i, j);
                    break;
                }
            }
        }
    }
}

static void donsol_game_clear_deltas(DonsolGame_t* game) {
    game->xpDelta = game->hpDelta = game->dpDelta = 0;
}

static void donsol_game_pick_potion(DonsolGame_t* game, Card_t *dcard, u8 val) {
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

static void donsol_game_pick_shield(DonsolGame_t* game, Card_t *dcard, int power) {
    donsol_game_clear_deltas(game);
    
    game->dpDelta = (s8)power - (s8)game->dp;
    game->dp = power;
    game->shieldBreakLimit = -1;
    
    game->xp++;
    game->xpDelta = 1;

    game->canDrink = 1;
}

static u8 donsol_game_pick_enemy(DonsolGame_t* game, Card_t *dcard, int atk) {
    donsol_game_clear_deltas(game);

    int damage = atk;
    if(game->dp > 0) {
        u8 shieldIsDamaged = game->shieldBreakLimit >= 0;
        if(shieldIsDamaged && atk > game->shieldBreakLimit) {
            game->dpDelta = -game->dp;
            game->dp = 0;
            game->shieldBreakLimit = -1;
            game->onStatusUpdate(DONSOL_SHIELD_BROKE, "Broke!");
        } else {
            game->shieldBreakLimit = atk;
            damage = atk > game->dp ? abs(atk - (int)game->dp) : 0;
        }
    }

    if(damage > 0) {
        game->hpDelta = -damage;
        game->hp -= damage;
    }

    DonsolCardDescription_t* cd = 0;
    for(u8 i = 0; i < 4; ++i) {
        if(dcard == game->slots[i].dcard) {
            cd = &game->slots[i];
            break;
        }
    }
    if(!cd) {
        game->onError("Couldnt find card description for enemy.");
        return 0;
    }

    if(game->hp < 1) {
        char msgBuff[32] = {0};
        sprintf(msgBuff, "The %s killed you!", cd->simpleName);
        game->wonOrDied = 1;
        game->onStatusUpdate(DONSOL_LOST_FIGHT, msgBuff);
        return 0;
    } else {
        char msgBuff[32] = {0};
        sprintf(msgBuff, "Battled the %s.", cd->simpleName);
        game->onStatusUpdate(DONSOL_LOST_FIGHT, msgBuff);
    }

    game->xp++;
    game->xpDelta = 1;

    game->canDrink = 1;
    game->canRun = 1;
    return 1;
}

static void donsol_check_for_stage_completion(DonsolGame_t* game) {
    for(int i = 0; i < 4; ++i) {
        if(game->slots[i].dcard != 0 && !donsol_card_IsFlipped(*game->slots[i].dcard)) {
            // at least one card left to deal with
            return;
        }
    }
    u8 anyRemaining = 0;
    for(int i = 0; i < g_DeckSize; ++i) {
        if(!donsol_card_IsFlipped(g_Deck[i])) {
            anyRemaining = 1;
            break;
        }
    }
    if(!anyRemaining) {
        game->wonOrDied = 1;
        game->onStatusUpdate(DONSOL_STATUS_YOU_WIN, "You win.");
    } else {
        donsol_game_deal4(game);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

void donsol_game_pick_run(DonsolGame_t* game) {
    if(game->wonOrDied) {
        donsol_game_start(game);
    }
    donsol_game_clear_deltas(game);

    if(!game->hasTakenAction) {
        donsol_game_deal4(game);
        return;
    }

    if(!game->canRun) {
        game->onStatusUpdate(DONSOL_STATUS_CANT_RUN, "You can't escape this room.");
        return;
    }

    donsol_game_deal4(game);

    game->canDrink = 1;
    game->canRun = 0;
}

void donsol_game_start(DonsolGame_t* game) {
    donsol_card_ClearFlippedBit(g_Deck, g_DeckSize);
    donsol_card_ShuffleDeck(g_Deck, g_DeckSize);
    // slots become invalid as soon as we shuffle anyways.
    memset(game->slots, 0, sizeof(game->slots));

    game->hp = 21;
    game->dp = 0;
    game->xp = 0;

    donsol_game_clear_deltas(game);

    game->canDrink = 1;
    game->canRun = 1;
    game->hasTakenAction = 0;

    game->wonOrDied = 0;

    donsol_game_set_slot(game, 0, 0);
    donsol_game_set_slot(game, 1, 1);
    donsol_game_set_slot(game, 2, 2);
    donsol_game_set_slot(game, 3, 3);

    game->onStatusUpdate(DONSOL_STATUS_WELCOME, "Entered Donsol.");
}

void donsol_game_pick_card(DonsolGame_t* game, u8 index) {
    if(game->wonOrDied) {
        return;
    }
    // from this point forward running has a cost.
    game->hasTakenAction = 1;

    if(index == 0 || index > 4) {
        if(game->onError) game->onError("pick card out of range.");
        return;
    }
    DonsolCardDescription_t* desc = &game->slots[index-1];
    Card_t *dcard = desc->dcard;
    
    // Couldn't find that card in the deck. This is an error case.
    if(0 == dcard) {
        if(game->onError) game->onError("Selected card has a null dcard");
        return;
    }

    if(*desc->dcard & CARDSTATE_FLIPPED) {
        donsol_game_clear_deltas(game);
        game->onStatusUpdate(DONSOL_STATUS_ALREADY_USED, "Can't do that again");
        return;
    }

    if(desc->isMonster) {
        if(!donsol_game_pick_enemy(game, dcard, desc->power)) {
            // You died, don't flip over the card that killed you,
            // and don't move to the next room if it was the last card
            return;
        }
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
    
    donsol_check_for_stage_completion(game);
}

void donsol_game_quit(DonsolGame_t* game) {

}