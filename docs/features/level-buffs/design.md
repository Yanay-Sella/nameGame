# Level buffs

**Status:** implemented · **Updated:** 2026-11-18 · [GDD](../../GDD.md) · [Rocket](../rocket/design.md) · [Bosses](../bosses/design.md) · [Implementation](implementation.md) · [Testing](testing.md)

Every big boss (waves 15 and 20) drops a buff for its level, and each level has its own ship skin. Buffs stack until the ship dies; three of them add a strong attack that R2 can fire instead of the rocket, and Square picks which one. Level 1's numbers and every enemy stay unchanged.

## Rules
1. **Skin:** each level draws the ship in its own look on the same 16×14 body and 6×6 hitbox. Space: today's ship. Jungle: wooden hull, leaf wings. Glacier: pale ice crystal. Volcano: slate obsidian with lava seams. Deep sea: a grey submarine with yellow portholes. Player 1 keeps the cyan cockpit; player 2 gets a gold copy with an orange cockpit.
2. **Drop:** a big boss's death (waves 15 and 20; each twin drops one) drops an 11×11 blinking yellow frame with the level's icon, falling at 30 px/s. The ship that touches it gets the buff; a missed pickup is gone. A buff already held pays 5,000 points.
3. **Stacking:** buffs from different levels stack, and in Classic they carry into the next level with the score. Dying loses every buff, the armed strong attack, a raised shield and a charging beam.
4. **Strong attack:** R2 fires the armed strong attack, by default the rocket. The space, glacier and deep-sea buffs each add one, and collecting one arms it at once. Square cycles rocket → magic beam → frost wave → bubble shield, skipping ones not held. Every strong attack costs 1 rocket and shares the rocket's 0.5 s cooldown and R2 buffer; with 0 rockets R2 plays the empty click.
5. **HUD:** held buffs show as 5×5 icons above the rocket icons, in level order; the armed strong attack's icon is underlined in yellow.
6. **Space (wand):** a wand sits on the ship's right wing. Magic beam: a ball grows at the wand's star for 0.8 s, then fires: the ball stays at the base and a 12 px beam shoots up to the top of the screen in 0.1 s and stays 0.4 s in all, throwing the odd spark, then breaks into small shards. When it reaches the top, every alien, boss and live fish touching the column takes 30 damage once. Cyan edges, white core, never magenta. It kicks the screen when it fires and rumbles while it's on.
7. **Jungle:** bullets are drawn as bananas. No other effect.
8. **Glacier (frost wave):** an ice ring sweeps out and every enemy freezes for 3 s (no moving, swaying, diving, shooting or attacking; still deadly to touch; still takes damage). Every living alien and boss heals 25 % of its max HP (rounded up). Every enemy projectile and in-flight attack is destroyed: shots, thrown hazards, lobs in flight, live fish, a charging or firing mage ball, beam or eruption, bursts, a gorilla pound or yeti sneeze wind-up and the fisherman's hook (a hooked ship is let go). Peels and stink mists on the ground stay. Clearing the wave ends the freeze. The wave shakes the screen like a rocket blast.
9. **Volcano:** bullets burn (flame bullets, fire bananas with the jungle buff) and fire 25 % faster. Damage is unchanged.
10. **Deep sea (bubble shield):** a bubble ring around the ship. The next deadly hit pops it instead of killing, gives 1 s of blinking invincibility and bursts 3 big bubbles. A bubble flies off in a random direction for 0.2 s, then homes on a random living enemy; a hit does 2 damage and splits into 3 bubbles one size smaller (8 px, 4 px, 2 px); a size-1 hit ends the chain: up to 39 hits, 78 damage. R2 with a shield up does nothing and spends no rocket. Non-lethal hazards (ice block, lava drop, hook, peels) don't touch the shield. The pop shakes the screen like a rocket blast.

## Why it stays fair
Buffs come only from big bosses and must be caught; all are lost on death; each strong attack spends a scarce rocket (pillar 2); the frost wave heals what it freezes; the shield needs a hit before it deals damage; the volcano buff adds speed, not damage.

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Buff pickup | 11×11, falls 30 | px, px/s | catchable before it leaves the screen |
| Held-buff payout | 5,000 | points | |
| Magic ball / beam | 0.8 / 0.4 (grows in 0.1), 12 wide | s, px | beam reads clearly |
| Magic beam damage | 30 | HP per enemy | a big boss takes a fair chunk, not a third |
| Frost freeze | 3 | s | long enough to matter, short enough to risk |
| Frost heal | 25 | % of max HP | a big boss gets back 85-105 HP |
| Volcano fire rate | interval × 0.8 | | |
| Shield pop invincibility | 1 | s | |
| Bubble damage / chain | 2 per hit, up to 39 hits | HP | 78 total; bubbles keep 60 fps |

## Edge cases
- A buff pickup dropped during the level-clear banner still falls and can be caught.
- A picker run (one level) keeps only that level's buff.
- Co-op: buffs, strong attack and shield are per player; the frost wave freezes the enemies for both.
- Square is button 8: the PS3 log showed `button 8` when Square was pressed, so the number is right. If it ever fails, R2 still fires the armed attack (the last collected strong buff).

## Open questions
- Do the buffs feel fair in a real run? (User's playtest.)

## History
- 2026-11-18: designed and implemented (D-030, D-031).
- 2026-11-18: beam build-up, shards and shake (D-032).
