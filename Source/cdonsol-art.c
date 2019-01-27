#include "cdonsol-art.h"

u16 cdonsol_art_width = 7;
u16 cdonsol_art_height = 4;

/*
          _____
         |A .  | _____
         | /.\ ||A ^  | _____
         |(_._)|| / \ ||A _  | _____
ejm98    |  |  || \ / || ( ) ||A_ _ |
         |____V||  .  ||(_'_)||( v )|
                |____V||  |  || \ / |
                       |____V||  .  |
                              |____V|

*/

char const* cdonsol_art_spade =
"   .   "
"  /.\\  "
" (_._) "
"   |   ";

char const* cdonsol_art_club =
"   _   "
"  ( )  "
"(_   _)"
"   |   ";

char const* cdonsol_art_joker =
" _____ "
"( ooo )"
"(_.V._)"
"{}{ }{}";

char const* cdonsol_art_heart =
"  ]o[  "
"  | |  "
" /---\\ "
"[_____]";

char const* cdonsol_art_diamond =
"  /-\\  "
"//---\\\\"
"\\\\---//"
"  \\-/  ";

char const* cdonsol_art_back =
".=====."
"|/////|"
"|\\\\\\\\\\|"
".=====.";
