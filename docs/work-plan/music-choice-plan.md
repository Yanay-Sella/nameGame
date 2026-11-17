# Plan: music setting MIZRAHIT / DUBSTEP / OFF

For the implementer: follow the steps in order. Every design choice below is final, so don't redesign anything.
Read only the files each step names, and don't re-read a file after editing it. Code style: follow `CLAUDE.md` and
the existing code (plain C++14, no new libraries, short one-line comments, no helpers for one-off logic).

**Branch:** work on a new branch, not `main`: `git switch -c music-choice` before step 1. The working tree may already hold
the user's uncommitted level-1 work (`docs/GDD.md`, `docs/features/level-1/`, `.claude/skills/...`): leave it as is.
**Don't commit.** The user commits; at the end, list the files you changed and suggest a one-line commit message.

## What the player gets
- Settings' first item rotates through three values instead of ON/OFF: `MUSIC: MIZRAHIT` → `MUSIC: DUBSTEP` → `MUSIC: OFF`
  → back to `MUSIC: MIZRAHIT`. X or D-pad right steps forward in that order; D-pad left steps backward. Each change is
  saved to settings.txt as today.
- MIZRAHIT is today's song (`music.ogg`). DUBSTEP is a new song (`dubstep.ogg`, next to EBOOT.BIN). OFF: no song;
  sound effects are unaffected.
- The chosen song starts with every run and loops, exactly where today's song starts; it stops at game over and on
  RETURN TO MENU. The main menu stays silent, and changing the setting from Settings opened from the main menu plays nothing.
- Changing it while paused switches at once: the new song starts from the beginning; OFF stops the music.
- Default (no settings.txt): MIZRAHIT. An old settings.txt keeps working: `music=1` → MIZRAHIT, `music=0` → OFF.
- If a song file is missing, choosing it plays nothing (silence, logged at boot), the other song still works.

## Step 1: settings (`source/settings.h`, `source/settings.cpp`)
In `settings.h`, above `struct Settings`:
```cpp
enum Music { MUSIC_OFF, MUSIC_MIZRAHIT, MUSIC_DUBSTEP, MUSIC_CHOICES };   // saved as music=0, 1, 2 (old files: 0 off, 1 on)
```
- `struct Settings`: replace `bool music = true;` with `Music music = MUSIC_MIZRAHIT;   // the song played during runs`.
- `settingsLoad`, the `music=` line: `settings.music = value >= 0 && value < MUSIC_CHOICES ? (Music)value : MUSIC_MIZRAHIT;`
  (out of range → default).
- `settingsSave`: write `music=%d` with `(int)settings.music` (replace `settings.music ? 1 : 0`).

## Step 2: audio (`source/audio.h`, `source/audio.cpp`)
`audio.h`:
- Add `#include "settings.h"` after `#include "game.h"`.
- `audioOpen` comment: `// folder holding music.ogg and dubstep.ogg; on failure the game runs silent`.
- Replace `void audioStartMusic();` with
  `void audioStartMusic(Music song);   // from the beginning, looping; MUSIC_OFF or a missing song stops the music`.

`audio.cpp`:
- First-line comment: `// SDL2_mixer: the songs (music.ogg, dubstep.ogg) play from memory; effects are synthesized once at startup into mixer chunks.`
- Replace `static Mix_Music* music = NULL;` with:
  ```cpp
  const char* const SONG_FILES[MUSIC_CHOICES] = {NULL, "music.ogg", "dubstep.ogg"};   // indexed by Music
  static Mix_Music* songs[MUSIC_CHOICES] = {};   // songs[MUSIC_OFF] stays NULL
  ```
- `audioOpen`: wrap the song-loading block (from `std::string path = ...` through the `if (!music) {...}` error block)
  in `for (int song = MUSIC_MIZRAHIT; song < MUSIC_CHOICES; song++) { ... }`, with `path = std::string(folder) + "/" + SONG_FILES[song]`
  and `songs[song]` in place of `music`. Keep the existing memory comment above the loop. Each song keeps its own
  `songBytes` for the game's lifetime (both fit easily: ~4 MB each).
- The final `audio:` log line becomes:
  ```cpp
  debugNetPrintf(INFO, "audio: %d Hz, format 0x%x, %d channels, mizrahit %s, dubstep %s\n", freq, format, channels,
                 songs[MUSIC_MIZRAHIT] ? "loaded" : "missing", songs[MUSIC_DUBSTEP] ? "loaded" : "missing");
  ```
- `audioStartMusic`:
  ```cpp
  void audioStartMusic(Music song) {
      if (!audioReady) return;
      if (!songs[song]) { Mix_HaltMusic(); return; }   // OFF, or the song didn't load: a song switched away from must stop too
      if (Mix_PlayMusic(songs[song], -1) != 0) debugNetPrintf(ERROR, "audio: Mix_PlayMusic failed: %s\n", Mix_GetError());
  }
  ```
- Leave `audioStopMusic`, `audioPlay`, `audioClose` as they are.

## Step 3: wiring (`source/main.cpp`)
- Next to `PAUSE_ITEMS`, add:
  ```cpp
  const char* const MUSIC_NAMES[] = {"off", "mizrahit", "dubstep"};                          // log names, indexed by Music
  const char* const MUSIC_LABELS[] = {"MUSIC: OFF", "MUSIC: MIZRAHIT", "MUSIC: DUBSTEP"};   // settings item, indexed by Music
  ```
- Startup `settings:` log: `music %s` with `MUSIC_NAMES[settings.music]`.
- `SCENE_MAIN_MENU`, CLASSIC: `if (settings.music) audioStartMusic();` → `audioStartMusic(settings.music);`.
- `SCENE_SETTINGS`, `if (selected == 0)` branch: replace its three lines with
  ```cpp
  settings.music = (Music)((settings.music + (menu.left ? MUSIC_CHOICES - 1 : 1)) % MUSIC_CHOICES);   // Mizrahit → Dubstep → Off → Mizrahit
  if (settingsBack == SCENE_PAUSED) audioStartMusic(settings.music);   // the main menu stays silent
  ```
- The `settings:` log after `settingsSave`: `music %s` with `MUSIC_NAMES[settings.music]`.
- `SCENE_PLAYING`, game-over new run: `if (settings.music) audioStartMusic();` → `audioStartMusic(settings.music);`
  (keep the `musicStart` timing and its log line).
- Drawing, `settingsItems`: first entry `MUSIC_LABELS[settings.music]`.
- Don't touch anything else (game-over `audioStopMusic()`, RETURN TO MENU, item counts, `drawMenu`).

## Step 4: song file and build (`Makefile`, `.gitignore`, `CLAUDE.md`)
- `Makefile`:
  - Above `music:`, add `MUSIC_OUT ?= music.ogg` and change the comment to:
    `# Convert a song to OGG for the package (the PS3's SDL2_mixer plays OGG and WAV only): make music MUSIC_SRC=path/to/song [MUSIC_OUT=dubstep.ogg]`
  - In `music`, the usage message: `"Usage: make music MUSIC_SRC=path/to/song [MUSIC_OUT=dubstep.ogg]"`; the ffmpeg output: `pkgfiles/USRDIR/$(MUSIC_OUT)`.
  - In `release`, add a second check after the `music.ogg` one:
    `@test -f pkgfiles/USRDIR/dubstep.ogg || { echo "pkgfiles/USRDIR/dubstep.ogg is missing; run make music MUSIC_OUT=dubstep.ogg first."; exit 1; }`
- `.gitignore`: add `pkgfiles/USRDIR/dubstep.ogg` under `pkgfiles/USRDIR/music.ogg` (the song isn't committed).
- Convert the song (the PS3 can't play MP3):
  ```bash
  make music MUSIC_SRC="/Users/yanaysella/Downloads/SKRILLEX - KYOTO (FT. SIRAH).mp3" MUSIC_OUT=dubstep.ogg
  ffprobe -v error -show_entries format=duration -of default=nw=1 pkgfiles/USRDIR/dubstep.ogg
  ```
  The duration must be about 200 s.
- `CLAUDE.md` › Commands, the `make music` line becomes:
  `` - `make music MUSIC_SRC=<song>` — convert a song to `pkgfiles/USRDIR/music.ogg` (MIZRAHIT); add `MUSIC_OUT=dubstep.ogg` for the DUBSTEP song. Both gitignored, packaged next to EBOOT.BIN. ``

## Step 5: tests (`tests/game_tests.cpp`)
- Rename `settingsDefaultToMusicOn` → `settingsDefaultToMizrahit`; it checks `settingsLoad("build/tests/missing.txt").music == MUSIC_MIZRAHIT`.
- `settingsSurviveASave`: for each of `MUSIC_OFF`, `MUSIC_DUBSTEP`, `MUSIC_MIZRAHIT` in that order, set `settings.music`,
  `CHECK(settingsSave("build/tests/settings.txt", settings))`, then `CHECK(settingsLoad("build/tests/settings.txt").music == <that value>)`.
- New `oldMusicSettingsStillLoad`, placed after `writeTextFile` (it uses it): write `"music=0\n"` → loads `MUSIC_OFF`;
  `"music=1\n"` → `MUSIC_MIZRAHIT`; `"music=7\n"` → `MUSIC_MIZRAHIT`. File: `build/tests/old-settings.txt`.
- `shakeAndFlashSettingsSurviveASave`: `loaded.music` → `loaded.music == MUSIC_MIZRAHIT`.
- `main()`: update the renamed call; call `oldMusicSettingsStillLoad()` after `settingsSurviveASave()`.
- `make test`: all must pass.

## Step 6: docs
Today is 2026-11-17. Edit only the lines named.
- `docs/features/screens/design.md`:
  - Rule 3: `MUSIC: ON / OFF` → `MUSIC: MIZRAHIT / DUBSTEP / OFF`, and the sentence about changing becomes:
    "X or D-pad right changes the item (music steps MIZRAHIT → DUBSTEP → OFF and wraps, shake steps up and wraps; left steps either one back)".
  - Rule 6: "OFF means no song plays; sound effects are unaffected. Changing the music while paused switches at once: a song starts from the beginning, OFF stops it. Changing it from the main menu plays nothing."
  - Edge cases: replace "Music toggled while paused..." with "An old settings.txt keeps working: `music=1` is MIZRAHIT, `music=0` is OFF."; in the defaults bullet "music on" → "music MIZRAHIT".
  - History: `- 2026-11-17: music setting chooses MIZRAHIT, DUBSTEP or OFF (D-017).`
- `docs/features/screens/implementation.md`: in the Persistence line, `music=` becomes "`music=` (0 off, 1 Mizrahit, 2 Dubstep)".
- `docs/features/screens/testing.md`: the two Mac criteria at the top become "Without a settings file, music defaults to MIZRAHIT. *(… › settingsDefaultToMizrahit)*" and "Every music choice saves and loads back; old `music=0/1` files still load. *(… › settingsSurviveASave, oldMusicSettingsStillLoad)*". Add unchecked: `- [ ] Settings rotates MUSIC: MIZRAHIT → DUBSTEP → OFF; a run plays the chosen song; switching while paused changes the song at once. *(not yet checked on the PS3)*`
- `docs/features/music-and-sound/design.md`:
  - Intro: "The user's song drives the action" → "One of two songs, chosen in Settings, drives the action"; "the only audio file is the song" → "the only audio files are the two songs".
  - Rule 1 becomes: "The chosen song (MIZRAHIT: `music.ogg`, DUBSTEP: `dubstep.ogg`, both next to EBOOT.BIN) starts when a run starts, loops, and stops at game over or when returning to the main menu; it keeps playing while paused. With OFF no song plays. The main menu is silent. Changing the choice while paused starts the new song from the beginning, or stops the music for OFF."
  - Rule 5: "If a song or the audio device fails, ..." (rest unchanged); Edge cases, add: "- A song file is missing: choosing it plays nothing; the other song still works."
  - History: `- 2026-11-17: second song (DUBSTEP) and a three-way music setting (D-017).`
- `docs/features/music-and-sound/implementation.md`: in the overview, "reads `music.ogg`" → "reads `music.ogg` and `dubstep.ogg`" and "plays it" → "plays the one chosen in settings"; the `main.cpp` table row: "plays requested sounds; the chosen song at a new run, off at game over"; Persistence: "The songs are `pkgfiles/USRDIR/music.ogg` and `dubstep.ogg`, made by `make music` (gitignored)."
- `docs/features/music-and-sound/testing.md`: add unchecked `- [ ] Both songs load on the PS3 and each plays in a run. *(not yet checked)*`
- `docs/GDD.md` (it may already hold the user's uncommitted 0.17 edits: keep them):
  - Header `**Version:** 0.17` (or whatever it shows) → `0.18`.
  - Feature index › Music and sound summary: "Two songs to choose from (Mizrahit, Dubstep) looped during gameplay; synthesized retro SFX".
  - Feature index › Screens summary: "settings (music, shake, flashing)" → "settings (music Mizrahit/Dubstep/Off, shake, flashing)".
  - Flow › Settings line: "music on/off" → "music Mizrahit / Dubstep / Off".
  - Audio line: "the user's song (an OGG copy of the FLAC) loops" → "the chosen song (Mizrahit or Dubstep, OGG copies) loops".
  - Decisions, after D-016: `| D-017 | 2026-11-17 | Music setting rotates MIZRAHIT, DUBSTEP, OFF; both songs are loaded into memory at boot; settings.txt keeps music=0/1 and adds 2 | music on/off with one song | the user wanted a second song to pick | [music and sound](features/music-and-sound/design.md), [screens](features/screens/design.md) |`
  - Change log, last line: `**0.18 (2026-11-17):** second song: music setting MIZRAHIT / DUBSTEP / OFF (D-017).`
- `README.md` › Troubleshooting: `**No music:** Settings → MUSIC: ON.` → `**No music:** Settings → MUSIC: MIZRAHIT or DUBSTEP.`
- Run `python3 .claude/skills/game-docs/scripts/check_docs.py docs` and fix what it reports in the files above.

## Step 7: PS3 check
Claude drives it (CLAUDE.md: `make launch`, `make pad`, `make shot`); only the listening needs the user.
1. `make ps3` must build without warnings in the changed code.
2. Start the log in the background into the scratchpad (not `build/`): `make log > <scratchpad>/ps3.log`.
   `make quit` if the game runs, then `make deploy`, then `make launch` and wait for `nameGame started`.
3. Log must show `audio: 48000 Hz, ... mizrahit loaded, dubstep loaded` and `settings: music mizrahit` (or `off` if the
   user's old file had `music=0`). Heap in the `fps` lines rises by ~4 MB versus before; fps stays ≈ 60.
4. Menu run, one `make pad` per line, checking the log after each:
   - `make pad BUTTONS='down|1|cross|1|cross'` → `scene: settings`, then `settings: music dubstep, ... (saved)`. `make shot` → item reads `MUSIC: DUBSTEP`.
   - `make pad BUTTONS='circle|1|up|1|cross'` → `scene: main menu`, `scene: playing`. Ask the user: is the dubstep song playing?
   - `make pad BUTTONS='start|1|down|1|cross|1|cross'` → `scene: paused`, `scene: settings`, `music off (saved)`. Ask: did the music stop?
   - `make pad BUTTONS='cross'` → `music mizrahit (saved)`. Ask: did Mizrahit start from the beginning?
   - `make pad BUTTONS='left'` → `music off (saved)`; `make pad BUTTONS='left'` → `music dubstep (saved)` (left steps back).
   - `make pad BUTTONS='right|1|right|1|circle|1|start'` → `music off`, `music mizrahit`, `scene: paused`, `scene: playing` (setting left at MIZRAHIT).
   These lines assume step 3 showed `music mizrahit`; if it showed `off`, run `make pad BUTTONS='down|1|cross|1|cross|1|circle'`
   first (off → mizrahit, saved; back on the main menu with SETTINGS selected), then run the first line as `make pad BUTTONS='cross|1|cross'`.
5. `make quit`, `make launch` → `settings: music mizrahit` (the choice survived).
6. If `dubstep missing` appears: check `pkgfiles/USRDIR/dubstep.ogg` exists before `make ps3`. If a song doesn't switch
   while paused, check that step 3 calls `audioStartMusic(settings.music)` only when `settingsBack == SCENE_PAUSED`.
7. Tick the new testing.md criteria that the log and the user's answers prove, with the date and the evidence.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean, on branch `music-choice`, nothing committed.
- On the PS3: Settings rotates MIZRAHIT → DUBSTEP → OFF, the chosen song plays in a run, switching while paused is immediate, and the choice survives a relaunch.
- Hand over: list the changed files and suggest the commit message `Music setting: MIZRAHIT, DUBSTEP or OFF`.
