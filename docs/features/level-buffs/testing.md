# Level buffs: testing

**Updated:** 2026-11-18 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Every skin is 16×14 and player 2's skins have no cyan cockpit. *(`tests/game_tests.cpp` › shipSkinsKeepTheBody)*
- [x] Big bosses drop one buff each; small bosses drop none. *(bigBossesDropTheLevelBuff)*
- [x] A buff is given once; a repeat pays 5,000; a strong-attack buff arms itself. *(buffPickupGivesTheBuffOnce)*
- [x] Dying loses every buff. *(dyingLosesEveryBuff)*
- [x] Square cycles the held strong attacks. *(squareCyclesHeldStrongAttacks)*
- [x] The magic beam hits its column once for 30. *(magicBeamHitsItsColumnOnce)*
- [x] The frost wave freezes, heals, shatters and thaws. *(frostWaveFreezesHealsAndShatters)*
- [x] The volcano buff fires 25 % faster. *(volcanoBuffFiresAQuarterFaster)*
- [x] The shield takes one hit and its bubbles chain for 78 damage. *(bubbleShieldTakesAHitAndChains)*
- [x] Classic keeps buffs into the next level. *(classicKeepsBuffsIntoTheNextLevel)*
- [ ] PS3: each level shows its ship skin.
- [ ] PS3: test mode (L2+L3) shows the wand, fire bananas and five HUD icons; log `player 1 buffs 0x1f`.
- [ ] PS3: Square cycles (log `strong attack ...`); R2 shows the beam column, the frost ring and the bubble ring.
- [ ] PS3: a shield pop and a later death log `shield popped`, `player 1 died` and `buffs 0x00`.
- [ ] PS3: autopilot run to wave 15 logs `buff pickup dropped: level 1 at wave 15`.
- [ ] PS3: 60 fps holds during a full bubble chain and a frost wave.
- [ ] One hand-played run judges whether the buffs feel fair (user's verdict).

## History
- 2026-11-18: criteria written.
