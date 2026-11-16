# Prototype: ship feel on the PS3

QUESTION:   Does flying and shooting with the PS3 pad feel tight on the real console, at a steady
            60 fps at 320x180 scaled up, with the song playing?
CORE VERB:  Move with the left stick / D-pad and hold R1 to shoot; tap R2 for a rocket.
THROWAWAY:  yes. One file, hard-coded values; the real game is written fresh in `source/` after a keep.
TIMEBOX:    one build-and-play session on the PS3 (about 2 hours of work).

BUILD:
- Grey boxes only: ship, bullets, rocket + explosion circle, 40 boxes swaying in a formation that
  drop shots. No art, menus, score or lives; hits only remove boxes.
- Everything drawn at 320x180 into a CPU pixel buffer, copied to the TV scaled by a whole number with
  one SDL_RenderCopy (the port's render targets are a no-op; see GDD D-008).
- The song as OGG, looping through SDL2_mixer; one synthesized beep on R2.
- Debug HUD: fps, frame time, object count, ship speed, fire interval.
- A "stress" button (Triangle, once its number is known) adds 100 bullets to test the frame rate.
- Logs every button press with its SDL index, so the controller mapping is measured.
- Tuning values as named constants at the top: ship speed, deadzone, fire interval, bullet speed,
  rocket speeds, explosion radius.

KEEP IF:    on the PS3 the user dodges and shoots without thinking about the controls within 30 s,
            fps stays at 60 with the stress button, the song plays without stutter, and the
            picture is sharp chunky pixels.
RETUNE IF:  one value feels off (ship too slow, fire rate too low): change that constant, rebuild.
KILL IF:    the scaled copy doesn't show or costs too much on the PS3: then draw every rect pre-scaled
            straight to the screen (decision for the GDD), and re-run this prototype.

RESULT:     2026-11-17 keep - the user played it on the PS3 and said it feels good. Log: 1920x1080 output,
            scale x6 with no borders, steady 60 fps at 1.0 ms work per frame (max 2.0 ms) with ~45
            objects, music.ogg loaded and played at 48 kHz, buttons 0 left / 2 right / 12 R1 confirmed.
            Not exercised in the session: R2 rocket, Triangle stress test, Start pause.
            Next: write the real game in source/ with these tuning values as starting numbers.
