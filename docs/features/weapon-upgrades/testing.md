# Weapon upgrades: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Levels 1–3 fire 1, 2 and 3 bullets; level 3 angles are −70, 0, +70 px/s; level 4 fires every 0.08 s. *(`tests/game_tests.cpp` › weaponLevelsFireTheirPattern)*
- [x] An upgrade pickup raises the level to at most 4; at 4 it gives 1,000 points and no rocket. *(`tests/game_tests.cpp` › upgradePickupRaisesLevelUpToFour)*
- [x] Dying loses one level, never below 1. *(`tests/game_tests.cpp` › deathLosesOneWeaponLevel)*
- [x] Each level is visible and distinct on the Mac render; the two pickups look different. *(Mac render 2026-11-17, scratchpad frames)*
- [x] On the PS3 the upgrades reach level 4 with a distinct pattern each, and dying drops a level. *(player report 2026-11-17; no log: the UDP port was held by another listener)*
- [x] Upgrade drop rate is about 3 % per kill (rocket about 2 %), and one roll never gives both pickups. *(`tests/game_tests.cpp` › killDropsFollowTheRoll: 20,000 kills)*
- [x] Level 4 is reached in a reasonable number of waves. *(player report 2026-11-17: numbers fine; PS3 log 2026-11-17: `weapon 4` by wave 8–11)*

## History
- 2026-11-17: criteria written.
