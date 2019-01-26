#include "donsol-card.h"

#ifdef _GU_H_
#   define donsolRandom guRandom
#else
#   include <stdlib.h>
#   define donsolRandom rand
#endif

void donsol_card_ClearFlippedBit(card_t* collection, u8 count)
{
  u8 i;
  for(i = 0; i < count; ++i)
  {
    collection[i] &= ~CARDSTATE_FLIPPED;
  }
}

void donsol_card_ShuffleDeck(card_t* collection, u8 count)
{
  u8 i, r;
  card_t t;
  for(i = 0; i < count; ++i)
  {
    r = donsolRandom() % count;
    t = collection[i];
    collection[i] = collection[r];
    collection[r] = t;
  }

}