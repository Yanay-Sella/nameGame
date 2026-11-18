// SDL2_mixer: the songs (music.ogg, dubstep.ogg) play from memory; effects are synthesized once at startup into mixer chunks.
#include "audio.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <debugnet.h>
#include <initializer_list>
#include <math.h>
#include <string>
#include <vector>

const int MUSIC_VOLUME = 96;        // of MIX_MAX_VOLUME (128)
const int MIXER_CHANNELS = 8;
const int SHOOT_CHANNEL = 0;        // reserved, so rapid fire never cuts off other sounds

// One part of an effect: a square wave or noise sweeping from startHz to endHz, fading out linearly.
struct Tone { float start, seconds, startHz, endHz, volume; bool noise; };

static bool audioReady = false;
const char* const SONG_FILES[MUSIC_CHOICES] = {NULL, "music.ogg", "dubstep.ogg"};   // indexed by Music
static Mix_Music* songs[MUSIC_CHOICES] = {};   // songs[MUSIC_OFF] stays NULL
static Mix_Chunk* sounds[SOUND_COUNT];

// Mixes the tones into one chunk in the mixer's own sample format (16-bit or float, as the PS3 device reports).
static Mix_Chunk* synthesize(std::initializer_list<Tone> tones) {
    int freq, channels;
    Uint16 format;
    Mix_QuerySpec(&freq, &format, &channels);
    float seconds = 0;
    for (const Tone& t : tones) seconds = fmaxf(seconds, t.start + t.seconds);
    int frames = (int)(seconds * freq);
    std::vector<float> mix(frames, 0.0f);
    uint32_t noiseState = 12345;
    for (const Tone& t : tones) {
        int first = (int)(t.start * freq), count = (int)(t.seconds * freq);
        float phase = 0, noiseValue = 0;
        for (int i = 0; i < count && first + i < frames; i++) {
            float progress = i / (float)count;
            float hz = t.startHz + (t.endHz - t.startHz) * progress;
            float before = phase;
            phase = fmodf(phase + hz / freq, 1.0f);
            float sample;
            if (t.noise) {   // sample-and-hold noise: a new random value each cycle, so hz sets its pitch
                if (phase < before) {
                    noiseState = noiseState * 1664525 + 1013904223;
                    noiseValue = (noiseState >> 8) / (float)0x800000 - 1;
                }
                sample = noiseValue;
            } else {
                sample = phase < 0.5f ? 1 : -1;
            }
            mix[first + i] += sample * t.volume * (1 - progress);
        }
    }
    int sampleBytes = format == AUDIO_F32SYS ? 4 : 2;
    Uint8* data = (Uint8*)SDL_malloc(frames * channels * sampleBytes);
    for (int i = 0; i < frames; i++) {
        float value = fmaxf(-1.0f, fminf(mix[i], 1.0f));
        for (int ch = 0; ch < channels; ch++) {
            if (sampleBytes == 4) ((float*)data)[i * channels + ch] = value;
            else ((Sint16*)data)[i * channels + ch] = (Sint16)(value * 32767);
        }
    }
    return Mix_QuickLoad_RAW(data, frames * channels * sampleBytes);
}

void audioOpen(const char* folder) {
    if (Mix_OpenAudio(48000, AUDIO_S16SYS, 2, 1024) != 0) {
        debugNetPrintf(ERROR, "audio: Mix_OpenAudio failed: %s\n", Mix_GetError());
        return;
    }
    int freq, channels;
    Uint16 format;
    Mix_QuerySpec(&freq, &format, &channels);
    if (format != AUDIO_S16SYS && format != AUDIO_F32SYS) {
        debugNetPrintf(ERROR, "audio: unsupported mixer format 0x%x, running silent\n", format);
        Mix_CloseAudio();
        return;
    }
    audioReady = true;
    Mix_AllocateChannels(MIXER_CHANNELS);
    Mix_ReserveChannels(1);   // channel 0 is SHOOT_CHANNEL
    Mix_VolumeMusic(MUSIC_VOLUME);

    // The song is read into memory once (~190 ms at boot): restarting it after a game over took ~350 ms when it
    // streamed from the hard drive and 1 ms from memory.
    for (int song = MUSIC_MIZRAHIT; song < MUSIC_CHOICES; song++) {
        std::string path = std::string(folder) + "/" + SONG_FILES[song];
        SDL_RWops* file = SDL_RWFromFile(path.c_str(), "rb");
        Sint64 size = file ? SDL_RWsize(file) : -1;
        void* songBytes = size > 0 ? SDL_malloc((size_t)size) : NULL;   // kept for the game's lifetime
        if (songBytes && SDL_RWread(file, songBytes, (size_t)size, 1) == 1)   // one object of `size` bytes
            songs[song] = Mix_LoadMUS_RW(SDL_RWFromConstMem(songBytes, (int)size), 1);
        if (file) SDL_RWclose(file);
        if (!songs[song]) {
            debugNetPrintf(ERROR, "audio: loading %s failed: %s\n", path.c_str(), Mix_GetError());
            SDL_free(songBytes);
        }
    }

    // Effects from music-and-sound/design.md: {start s, length s, start Hz, end Hz, volume, noise}.
    sounds[SOUND_SHOOT] = synthesize({{0, 0.05f, 1400, 900, 0.12f, false}});
    sounds[SOUND_ALIEN_HIT] = synthesize({{0, 0.04f, 500, 380, 0.2f, false}});
    sounds[SOUND_ALIEN_DEATH] = synthesize({{0, 0.15f, 4000, 1000, 0.3f, true}});
    sounds[SOUND_ROCKET_LAUNCH] = synthesize({{0, 0.3f, 250, 900, 0.3f, false}});
    sounds[SOUND_EXPLOSION] = synthesize({{0, 0.6f, 1200, 200, 0.5f, true}});
    sounds[SOUND_PICKUP] = synthesize({{0, 0.06f, 660, 660, 0.3f, false}, {0.06f, 0.06f, 990, 990, 0.3f, false}});
    sounds[SOUND_PLAYER_DEATH] = synthesize({{0, 0.8f, 3000, 300, 0.5f, true}, {0, 0.8f, 600, 80, 0.3f, false}});
    sounds[SOUND_ROCKET_EMPTY] = synthesize({{0, 0.03f, 150, 150, 0.2f, false}});
    sounds[SOUND_WAVE_START] = synthesize({{0, 0.08f, 523, 523, 0.25f, false}, {0.08f, 0.08f, 659, 659, 0.25f, false},
                                           {0.16f, 0.08f, 784, 784, 0.25f, false}});
    sounds[SOUND_EXTRA_LIFE] = synthesize({{0, 0.07f, 523, 523, 0.3f, false}, {0.07f, 0.07f, 659, 659, 0.3f, false},
                                           {0.14f, 0.07f, 784, 784, 0.3f, false}, {0.21f, 0.07f, 1047, 1047, 0.3f, false}});
    sounds[SOUND_BANANA_THROW] = synthesize({{0, 0.12f, 2000, 600, 0.2f, true}});
    sounds[SOUND_SCREECH] = synthesize({{0, 0.08f, 1400, 1900, 0.2f, false}, {0.08f, 0.08f, 1900, 1300, 0.2f, false},
                                        {0.18f, 0.08f, 1500, 2000, 0.2f, false}});
    sounds[SOUND_SLIP] = synthesize({{0, 0.35f, 1400, 300, 0.3f, false}});
    sounds[SOUND_STUN] = synthesize({{0, 0.12f, 200, 450, 0.3f, false}, {0.12f, 0.15f, 450, 180, 0.3f, false}});
    sounds[SOUND_SPLAT] = synthesize({{0, 0.25f, 700, 90, 0.4f, true}});
    sounds[SOUND_LAUGH] = synthesize({{0, 0.09f, 330, 260, 0.35f, false}, {0.13f, 0.09f, 300, 240, 0.35f, false},
                                      {0.26f, 0.09f, 270, 210, 0.35f, false}});
    sounds[SOUND_POUND] = synthesize({{0, 0.1f, 300, 80, 0.45f, true}, {0.2f, 0.1f, 300, 80, 0.45f, true},
                                      {0.4f, 0.15f, 250, 60, 0.5f, true}});
    sounds[SOUND_SNEEZE] = synthesize({{0, 0.25f, 4000, 900, 0.4f, true}, {0, 0.1f, 900, 400, 0.25f, false}});
    sounds[SOUND_FREEZE] = synthesize({{0, 0.3f, 3500, 5000, 0.25f, true}, {0, 0.3f, 1800, 2600, 0.15f, false}});
    sounds[SOUND_CRACK] = synthesize({{0, 0.06f, 5000, 3000, 0.3f, true}, {0.08f, 0.06f, 4500, 2500, 0.25f, true}});
    sounds[SOUND_SIZZLE] = synthesize({{0, 0.4f, 6000, 3000, 0.25f, true}});
    sounds[SOUND_BUBBLE] = synthesize({{0, 0.08f, 300, 900, 0.25f, false}, {0.09f, 0.06f, 500, 1100, 0.2f, false}});
    sounds[SOUND_CAST] = synthesize({{0, 0.2f, 2500, 500, 0.2f, true}, {0, 0.2f, 1200, 400, 0.15f, false}});
    sounds[SOUND_HOOKED] = synthesize({{0, 0.08f, 800, 800, 0.3f, false}, {0.1f, 0.08f, 1000, 1000, 0.3f, false},
                                       {0.2f, 0.15f, 600, 1200, 0.3f, false}});
    debugNetPrintf(INFO, "audio: %d Hz, format 0x%x, %d channels, mizrahit %s, dubstep %s\n", freq, format, channels,
                   songs[MUSIC_MIZRAHIT] ? "loaded" : "missing", songs[MUSIC_DUBSTEP] ? "loaded" : "missing");
}

void audioStartMusic(Music song) {
    if (!audioReady) return;
    if (!songs[song]) { Mix_HaltMusic(); return; }   // OFF, or the song didn't load: a song switched away from must stop too
    if (Mix_PlayMusic(songs[song], -1) != 0) debugNetPrintf(ERROR, "audio: Mix_PlayMusic failed: %s\n", Mix_GetError());
}

void audioStopMusic() {
    if (audioReady) Mix_HaltMusic();
}

void audioPlay(Sound sound) {
    if (audioReady && sounds[sound]) Mix_PlayChannel(sound == SOUND_SHOOT ? SHOOT_CHANNEL : -1, sounds[sound], 0);
}

void audioClose() {
    if (!audioReady) return;
    Mix_HaltMusic();
    Mix_CloseAudio();
    audioReady = false;
}
