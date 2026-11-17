// The song and the synthesized sound effects (docs/features/music-and-sound).
#pragma once
#include "game.h"
#include "settings.h"

void audioOpen(const char* folder);   // folder holding music.ogg and dubstep.ogg; on failure the game runs silent
void audioStartMusic(Music song);   // from the beginning, looping; MUSIC_OFF or a missing song stops the music
void audioStopMusic();
void audioPlay(Sound sound);
void audioClose();
