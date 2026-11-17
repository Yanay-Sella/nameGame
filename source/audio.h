// The song and the synthesized sound effects (docs/features/music-and-sound).
#pragma once
#include "game.h"

void audioOpen(const char* folder);   // folder holding music.ogg; on failure the game runs silent
void audioStartMusic();               // from the beginning, looping
void audioStopMusic();
void audioPlay(Sound sound);
void audioClose();
