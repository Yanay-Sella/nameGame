// SDL2_mixer: the songs (music.ogg, dubstep.ogg) play from memory; effects are synthesized once at startup into mixer chunks.
#include "audio.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <debugnet.h>
#include <math.h>
#include <string>
#include <vector>

const int MUSIC_VOLUME = 96;        // of MIX_MAX_VOLUME (128)
const int MIXER_CHANNELS = 8;
const int SHOOT_CHANNEL = 0;        // reserved, so rapid fire never cuts off other sounds
const int AMBIENT_CHANNEL = 1;      // reserved for NAME QUEST's ambience loops (spec §16)
const int AMBIENT_COUNT = 9;        // 0 off, 1 Namula, 2-8 worlds 1-7
const float AMBIENT_SECONDS = 4;

// One part of an effect: a square wave or noise sweeping from startHz to endHz; fade 1 fades out linearly, 0 stays flat.
struct Tone { float start, seconds, startHz, endHz, volume; bool noise; float fade = 1; };

static bool audioReady = false;
const char* const SONG_FILES[MUSIC_CHOICES] = {NULL, "music.ogg", "dubstep.ogg"};   // indexed by Music
static Mix_Music* songs[MUSIC_CHOICES] = {};   // songs[MUSIC_OFF] stays NULL
static Mix_Chunk* sounds[SOUND_COUNT];
static Mix_Chunk* ambients[AMBIENT_COUNT];   // ambients[0] stays NULL
// NAME QUEST songs (D-057): one per place (index = ambience id) and the boss song last; loaded on first use.
const int QUEST_SONGS = AMBIENT_COUNT + 1, BOSS_SONG = AMBIENT_COUNT;
const char* const QUEST_SONG_FILES[QUEST_SONGS] = {NULL, "quest_namula.ogg", "quest_world1.ogg", "quest_world2.ogg",
    "quest_world3.ogg", "quest_world4.ogg", "quest_world5.ogg", "quest_world6.ogg", "quest_world7.ogg", "quest_boss.ogg"};
static Mix_Music* questSongs[QUEST_SONGS];
static bool questSongTried[QUEST_SONGS];
static std::string songFolder;
static int questPlace = 0, questSong = -1;   // what audioSetQuestSound last set up; questSong -1: no quest song playing
static bool questBoss = false, questMusicOn = false;
static int ambientPlaying = 0;   // the ambience loop on channel 1, 0 none

// Mixes the tones into one chunk in the mixer's own sample format (16-bit or float, as the PS3 device reports),
// at least minSeconds long (ambience loops keep their full length even when their last sound ends early).
static Mix_Chunk* synthesize(const std::vector<Tone>& tones, float minSeconds = 0) {
    int freq, channels;
    Uint16 format;
    Mix_QuerySpec(&freq, &format, &channels);
    float seconds = minSeconds;
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
            mix[first + i] += sample * t.volume * (1 - progress * t.fade);
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

// Reads an OGG into memory (kept for the game's lifetime) and opens it as music; NULL when missing or unreadable.
static Mix_Music* loadSong(const std::string& path) {
    SDL_RWops* file = SDL_RWFromFile(path.c_str(), "rb");
    Sint64 size = file ? SDL_RWsize(file) : -1;
    void* songBytes = size > 0 ? SDL_malloc((size_t)size) : NULL;
    Mix_Music* music = NULL;
    if (songBytes && SDL_RWread(file, songBytes, (size_t)size, 1) == 1)   // one object of `size` bytes
        music = Mix_LoadMUS_RW(SDL_RWFromConstMem(songBytes, (int)size), 1);
    if (file) SDL_RWclose(file);
    if (!music) SDL_free(songBytes);
    return music;
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
    songFolder = folder;
    Mix_AllocateChannels(MIXER_CHANNELS);
    Mix_ReserveChannels(2);   // channel 0 is SHOOT_CHANNEL, 1 AMBIENT_CHANNEL
    Mix_VolumeMusic(MUSIC_VOLUME);

    // The song is read into memory once (~190 ms at boot): restarting it after a game over took ~350 ms when it
    // streamed from the hard drive and 1 ms from memory.
    for (int song = MUSIC_MIZRAHIT; song < MUSIC_CHOICES; song++) {
        songs[song] = loadSong(std::string(folder) + "/" + SONG_FILES[song]);
        if (!songs[song]) debugNetPrintf(ERROR, "audio: loading %s failed: %s\n", SONG_FILES[song], Mix_GetError());
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
    // NAME QUEST (spec §16): footsteps per surface, then the hero's and the world's sounds.
    sounds[SOUND_Q_STEP_STONE] = synthesize({{0, 0.03f, 900, 700, 0.08f, true}});
    sounds[SOUND_Q_STEP_METAL] = synthesize({{0, 0.03f, 2600, 2000, 0.07f, true}, {0, 0.02f, 1200, 1200, 0.04f, false}});
    sounds[SOUND_Q_STEP_LEAVES] = synthesize({{0, 0.03f, 3000, 1800, 0.06f, true}});
    sounds[SOUND_Q_STEP_SNOW] = synthesize({{0, 0.03f, 1800, 1200, 0.09f, true}});
    sounds[SOUND_Q_STEP_ROCK] = synthesize({{0, 0.03f, 600, 400, 0.10f, true}});
    sounds[SOUND_Q_STEP_SAND] = synthesize({{0, 0.03f, 700, 500, 0.08f, true}});
    sounds[SOUND_Q_STEP_MARBLE] = synthesize({{0, 0.03f, 2200, 2000, 0.06f, true}, {0, 0.02f, 1600, 1600, 0.03f, false}});
    sounds[SOUND_Q_STEP_PAPER] = synthesize({{0, 0.03f, 4000, 3000, 0.03f, true}});
    sounds[SOUND_Q_JUMP] = synthesize({{0, 0.08f, 300, 600, 0.15f, false}});
    sounds[SOUND_Q_LAND] = synthesize({{0, 0.06f, 500, 200, 0.15f, true}});
    sounds[SOUND_Q_SWING] = synthesize({{0, 0.09f, 2500, 900, 0.2f, true}});
    sounds[SOUND_Q_STAB] = synthesize({{0, 0.05f, 3500, 2000, 0.18f, true}});
    sounds[SOUND_Q_ARROW] = synthesize({{0, 0.07f, 1800, 1200, 0.12f, false}});
    sounds[SOUND_Q_SPELL] = synthesize({{0, 0.15f, 600, 1400, 0.15f, false}, {0, 0.15f, 3000, 5000, 0.08f, true}});
    sounds[SOUND_Q_HIT] = synthesize({{0, 0.05f, 1200, 600, 0.3f, true}});
    sounds[SOUND_Q_CRIT] = synthesize({{0, 0.05f, 1200, 600, 0.3f, true}, {0, 0.06f, 1500, 2200, 0.2f, false}});
    sounds[SOUND_Q_HURT] = synthesize({{0, 0.15f, 400, 150, 0.35f, false}, {0, 0.1f, 1500, 500, 0.3f, true}});
    sounds[SOUND_Q_DEATH] = synthesize({{0, 0.9f, 2500, 200, 0.45f, true}, {0, 0.9f, 500, 60, 0.3f, false}});
    sounds[SOUND_Q_NAMES] = synthesize({{0, 0.05f, 880, 880, 0.2f, false}, {0.05f, 0.05f, 1175, 1175, 0.2f, false},
                                        {0.1f, 0.05f, 1568, 1568, 0.2f, false}});
    sounds[SOUND_Q_KINDLE] = synthesize({{0, 0.4f, 800, 2500, 0.3f, true}, {0.1f, 0.3f, 220, 440, 0.2f, false}});
    sounds[SOUND_Q_LEVEL_UP] = synthesize({{0, 0.09f, 523, 523, 0.25f, false}, {0.09f, 0.09f, 659, 659, 0.25f, false},
                                           {0.18f, 0.09f, 784, 784, 0.25f, false}, {0.27f, 0.09f, 1047, 1047, 0.25f, false},
                                           {0.36f, 0.09f, 1319, 1319, 0.25f, false}});
    sounds[SOUND_Q_PORTAL] = synthesize({{0, 0.3f, 200, 1200, 0.2f, false}, {0, 0.3f, 210, 1210, 0.15f, false}});
    sounds[SOUND_Q_BLIP] = synthesize({{0, 0.03f, 1000, 1000, 0.15f, false}});
    sounds[SOUND_Q_TALK] = synthesize({{0, 0.02f, 700, 700, 0.08f, false}});
    sounds[SOUND_Q_DRINK] = synthesize({{0, 0.05f, 400, 600, 0.15f, false}, {0.1f, 0.05f, 400, 600, 0.15f, false},
                                        {0.2f, 0.05f, 400, 600, 0.15f, false}});
    sounds[SOUND_Q_DODGE] = synthesize({{0, 0.12f, 1500, 3000, 0.15f, true}});
    sounds[SOUND_Q_SPLASH] = synthesize({{0, 0.3f, 1500, 300, 0.4f, true}});
    sounds[SOUND_Q_ZAP] = synthesize({{0, 0.15f, 6000, 4000, 0.3f, true}, {0, 0.15f, 120, 120, 0.2f, false}});
    sounds[SOUND_Q_ROAR] = synthesize({{0, 0.6f, 300, 120, 0.5f, true}, {0, 0.6f, 110, 70, 0.3f, false}});
    sounds[SOUND_Q_GATE] = synthesize({{0, 0.5f, 200, 100, 0.4f, true}});
    sounds[SOUND_Q_CHEST] = synthesize({{0, 0.06f, 660, 660, 0.25f, false}, {0.06f, 0.06f, 990, 990, 0.25f, false}});
    // NAME QUEST ambience (spec §16): 4 s loops, each under volume 0.12 in total. Flat tones span the whole loop with a
    // whole number of cycles; short sounds sit inside it, so the loop is seamless.
    std::vector<Tone> loop;
    uint32_t seed = 777;
    auto random01 = [&seed]() { seed = seed * 1664525u + 1013904223u; return (seed >> 8) / 16777216.0f; };
    auto scatter = [&](int count, float seconds, float fromHz, float toHz, float volume, bool noise) {
        for (int i = 0; i < count; i++) loop.push_back({random01() * (AMBIENT_SECONDS - seconds), seconds, fromHz, toHz, volume, noise});
    };
    const float S = AMBIENT_SECONDS, H = AMBIENT_SECONDS / 2;
    // Namula and world 1 had a low square-wave hum (70 / 55 Hz) under them; TV speakers drop its low tone and keep its
    // harsh harmonics, which the user heard as white noise on the PS3, so it was removed (2026-11-21).
    loop = {};                                                     // Namula: the hearth's crackle
    scatter(36, 0.03f, 2500, 600, 0.06f, true);
    ambients[1] = synthesize(loop, S);
    loop = {};                                                     // world 1: a station's clicks
    for (int i = 0; i < 8; i++) loop.push_back({i * 0.5f + 0.13f, 0.01f, 4000, 3500, 0.04f, true});
    ambients[2] = synthesize(loop, S);
    loop = {};                                                     // world 2: chirps
    scatter(10, 0.06f, 3000, 3600, 0.05f, false);
    ambients[3] = synthesize(loop, S);
    loop = {{0, H, 400, 900, 0.06f, true, 0}, {H, H, 900, 400, 0.06f, true, 0}};   // world 3: wind swells
    ambients[4] = synthesize(loop, S);
    loop = {{0, H, 80, 200, 0.08f, true, 0}, {H, H, 200, 80, 0.08f, true, 0}};     // world 4: rumble
    ambients[5] = synthesize(loop, S);
    loop = {{0, S, 300, 300, 0.04f, true, 0}};                      // world 5: water and bubbles
    scatter(8, 0.08f, 300, 900, 0.05f, false);
    ambients[6] = synthesize(loop, S);
    loop = {{0, H, 500, 900, 0.05f, true, 0}, {H, H, 900, 500, 0.05f, true, 0}, {0, 1.5f, 660, 660, 0.05f, false}};   // world 6: wind, a bell
    ambients[7] = synthesize(loop, S);
    loop = {{0, S, 2000, 2000, 0.015f, true, 0}};                   // world 7: near silence and a pen's scratch
    scatter(4, 0.15f, 5000, 3000, 0.03f, true);
    ambients[8] = synthesize(loop, S);
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

void audioSetQuestSound(int place, bool boss, bool musicOn) {
    if (!audioReady || place < 0 || place >= AMBIENT_COUNT) return;
    if (place == questPlace && boss == questBoss && musicOn == questMusicOn) return;
    questPlace = place;
    questBoss = boss;
    questMusicOn = musicOn;
    // The song wanted: the boss song in a fight, else the place's; each file is tried once, a missing one is logged.
    int want = place == 0 || !musicOn ? -1 : boss ? BOSS_SONG : place;
    if (want >= 0 && !questSongTried[want]) {
        questSongTried[want] = true;
        questSongs[want] = loadSong(songFolder + "/" + QUEST_SONG_FILES[want]);
        debugNetPrintf(INFO, "audio: %s %s\n", QUEST_SONG_FILES[want], questSongs[want] ? "loaded" : "missing");
    }
    if (want == BOSS_SONG && !questSongs[want]) want = questSongs[place] ? place : -1;   // no boss song: the place's
    if (want >= 0 && !questSongs[want]) want = -1;
    if (want != questSong) {
        if (want >= 0) Mix_PlayMusic(questSongs[want], -1);
        else if (questSong >= 0) Mix_HaltMusic();   // only a quest song is stopped, never the shooter's
        questSong = want;
    }
    // The ambience plays only where no song does; it keeps running across a fight's start and end.
    int ambience = want < 0 && ambients[place] ? place : 0;
    if (ambience != ambientPlaying) {
        if (ambience) Mix_PlayChannel(AMBIENT_CHANNEL, ambients[ambience], -1);
        else Mix_HaltChannel(AMBIENT_CHANNEL);
        ambientPlaying = ambience;
    }
}

void audioClose() {
    if (!audioReady) return;
    Mix_HaltMusic();
    Mix_CloseAudio();
    audioReady = false;
}
