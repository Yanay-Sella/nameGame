# Rocket

**Status:** implemented · **Updated:** 2026-11-17 · [GDD](../../GDD.md) · [Implementation](implementation.md) · [Testing](testing.md)

R2 launches a big rocket that climbs, speeds up and explodes in a large blast on the first thing it touches. It's the panic button (pillar 2): rare, loud and decisive, so the player saves it for a crowded moment or a boss.

## Rules
1. R2 (`rocket`, read as a press) launches one rocket from the ship's nose if the player has one and no cooldown is running.
2. A press that arrives while the rocket can't fire yet is buffered and fires as soon as it can, within the buffer window; the buffer clears on death and pause.
3. The rocket accelerates straight up and explodes on touching an enemy, an enemy's body part or the top of the screen. It ignores enemy shots.
4. The explosion damages every enemy inside its radius once.
5. Rocket pickups drop from killed enemies and fall down; touching one adds a rocket, up to the maximum.
6. The HUD shows the rocket count as icons.
7. Dying doesn't remove rockets.
8. R2 fires the armed strong attack: the rocket, or a level buff's ([level buffs](../level-buffs/design.md)).

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Starting rockets | 3 | rockets | still have one when a wave goes bad [ASSUMPTION] |
| Maximum rockets | 9 | rockets | [ASSUMPTION] |
| Cooldown between rockets | 0.5 | s | [ASSUMPTION] |
| Input buffer | 0.15 | s | |
| Launch speed → top speed | 60 → 260 | px/s | launch feels heavy, then fast [ASSUMPTION] |
| Explosion radius | 40 | px | clears a chunk of a formation, not all of it [ASSUMPTION] |
| Explosion damage | 20 | HP | kills normal aliens; hurts a boss a lot [ASSUMPTION] |
| Pickup drop chance | 2 | % per kill | about one rocket per 2 waves [ASSUMPTION] |
| Pickup fall speed | 30 | px/s | reachable before it leaves the screen [ASSUMPTION] |
| Blast on screen | grows 0.1, dissolves 0.35 | s | [ASSUMPTION] |

## Edge cases
- Two rockets in flight: each explodes on its own; an enemy is damaged by each blast at most once.
- Pause mid-flight: rocket and explosion freeze; the buffer clears.
- Ship dies after launch: the rocket keeps flying.
- R2 spam with 0 rockets: nothing fires, a short "empty" click plays.

## History
- 2026-11-17: designed.
- 2026-11-17: implemented in M2.
