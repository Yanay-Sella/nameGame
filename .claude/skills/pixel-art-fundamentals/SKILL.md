---
name: pixel-art-fundamentals
description: Review or refine existing NAME QUEST pixel-art characters, enemies, and poses for readable silhouettes, shading, and consistent proportions at native size. Use sprite-animation for timing, cycles, and transitions.
---

# Pixel art fundamentals

Read [NAME QUEST spec §22](../../../docs/work-plan/name-quest/spec.md#22-art-direction-rich-pixel-art-at-640x360) and compare the sprite with neighboring art of the same kind. Use the existing proportions, palette, pixel density, and outline conventions for that group. For heroes and humanoid NPCs, follow their shared 32×28 art-pixel grid and flat, outline-free style.

- Read the outer shape first. Check that the head, torso, limbs, and the important weapon or gesture are identifiable at native size. Move a limb or open a small gap if overlapping shapes hide the pose.
- Group neighboring pixels into clear shapes for lit and shaded planes. Review isolated pixels and parallel bands for noise, especially when they flicker between poses. Use the established light direction and palette.
- Compare adjacent animation frames at a fixed anchor. Check that head size, torso mass, and weapon length stay consistent as the silhouette changes.
- Capture and open a Mac screenshot after visual edits. Judge the pose at the game's 640×360 render resolution against actual backgrounds and neighboring sprites. Use zoom to diagnose individual pixels.

Adapted for NAME QUEST from [ProkStudio Pixel Art Fundamentals](https://github.com/ProkStudio/skills/blob/main/skills/game/2d/pixel-art-fundamentals/SKILL.md).
