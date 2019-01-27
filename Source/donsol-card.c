#include "donsol-card.h"

#ifdef _GU_H_
#   define donsolRandom guRandom
#else
#   include <stdlib.h>
#   define donsolRandom rand
#endif

u8 donsol_card_IsJoker(card_t card)
{
  return (card & CARD_MASK) >= CARD_JO1;
}

u8 donsol_card_IsFace(card_t card)
{
  card = card & CARD_MASK;
  return card >= CARD_J && CARD_J <= CARD_K;
}

u8 donsol_card_IsNumeric(card_t card)
{
  card &= CARD_MASK;
  return card > CARD_A && card <= CARD_10;
}

u8 donsol_card_GetNumericValue(card_t card)
{
  return card & CARD_MASK;
}

u8 donsol_card_GetSuit(card_t card)
{
  return (card & SUIT_MASK);
}

u8 donsol_card_IsHearts(card_t card)
{
  return donsol_card_GetSuit(card) == SUIT_HEARTS;
}

u8 donsol_card_IsDiamonds(card_t card)
{
  return donsol_card_GetSuit(card) == SUIT_DIAMONDS;
}

u8 donsol_card_IsSpades(card_t card)
{
  return donsol_card_GetSuit(card) == SUIT_SPADES;
}

u8 donsol_card_IsClubs(card_t card)
{
  return donsol_card_GetSuit(card) == SUIT_CLUBS;
}

u8 donsol_card_IsFlipped(card_t card)
{
  return !!(card & CARDSTATE_FLIPPED);
}

void donsol_card_ClearFlippedBit(card_t* collection, u8 count) {
  u8 i;
  for(i = 0; i < count; ++i) {
    collection[i] &= ~CARDSTATE_FLIPPED;
  }
}

void donsol_card_ShuffleDeck(card_t* collection, u8 count) {
  u8 i, r;
  card_t t;
  for(i = 0; i < count; ++i) {
    r = donsolRandom() % count;
    t = collection[i];
    collection[i] = collection[r];
    collection[r] = t;
  }

}