---
name: sprite-animation
description: Review and improve NAME QUEST sprite motion, including stiff or sliding walk cycles, abrupt starts and stops, jump and landing poses, and hard-to-read attacks. Use for key poses, frame timing, anchors, and transitions. Use game-feel for impact effects and input-systems for dropped inputs.
---

# Sprite animation

Start with the current animation in play. Read the character art rules in [NAME QUEST spec §22](../../../docs/work-plan/name-quest/spec.md#22-art-direction-rich-pixel-art-at-640x360) and inspect `source/quest_draw.cpp` alongside the gameplay code that supplies its movement and attack state. Use the current spec for attack poses, frame counts, and gameplay timing.

1. Identify the visible problem. Compare motion at native size and frame by frame. Check whether the problem comes from the drawings, their playback, or the position and state driving them.
2. For locomotion, compare contact, passing, and lift poses. During a planted step, check the foot against the floor as the character moves. Match the stride cadence to actual travel and check the transition into idle, a quick reversal, and both facings for sliding or anchor jumps.
3. For airborne motion, check takeoff, fall, and ground contact against the actual movement state. Use the pose to show the change in direction or weight.
4. For attacks, check the wind-up, contact, and recovery silhouettes first. Relate the visible contact to the existing active window. Vary visual pose holds where the spec allows, preserving input response, hit windows, and combo rules. Check individual R1 strikes and a queued chain.
5. Keep the character's ground anchor consistent across drawings and mirroring. Add intermediate poses, cloth or weapon follow-through where the motion needs them and the existing frame budget allows.
6. Verify in `make mac-run` at normal speed on a real stage. Use scripted inputs for the transition you changed, then capture and inspect an L1 screenshot of a representative pose. Use playback to judge timing and the still to check silhouette and alignment.

Adapted for NAME QUEST from [ProkStudio Sprite Animation](https://github.com/ProkStudio/skills/blob/main/skills/game/2d/sprite-animation/SKILL.md).
