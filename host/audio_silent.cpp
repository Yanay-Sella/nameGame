// Mac build: no sound (the PS3 build's audio.cpp needs SDL2_mixer).
#include "../source/audio.h"

void audioOpen(const char*) {}
void audioStartMusic(Music) {}
void audioStopMusic() {}
void audioPlay(Sound) {}
void audioClose() {}
