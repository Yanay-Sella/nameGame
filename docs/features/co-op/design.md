# Co-op

**Status:** implemented · **Updated:** 2026-11-21 · [GDD](../../GDD.md)

Classic offers 1–2 Players and 1–4 Players. Both play the same five worlds and boss-wave schedule. The 1–4 player arena gives the party more room and tougher waves. See [Screens](../screens/design.md) for the controller lobby.

## Rules
1. Classic 1–2 Players and the Level Picker start with one ship. Classic 1–4 Players starts with the party chosen in the lobby. During play, Start or X on a spare controller adds a ship up to the variant's limit.
2. A joining player starts with 3 lives, 3 rockets and 0 points. They arrive blinking as after a respawn.
3. Each player has their own lives, rockets, score and extra lives. Bullet and rocket kills score for the player who fired. Pickups go to whoever touches them.
4. In Classic 1–2 Players, clearing a boss wave while a teammate still has lives brings back an eliminated player with 3 lives (two spare hearts). This happens when the last boss falls. The ship returns at its usual spawn with normal respawn invincibility. Its score, rockets and current weapon level carry over.
5. The wave clear bonus goes to every player still in the run.
6. Any joined player's Start pauses. A disconnect pauses until that player's PS3 port is connected again. Players then resume manually. See [Screens](../screens/design.md) for replay.
7. Each player has a score, lives, upgrades and rockets display. In 1–4 Players, all four scores have space across the screen. Player 2's hull is gold, player 3's cyan and player 4's purple. Every ship keeps its level skin and hitbox.

## Tuning by variant

| Rule | 1–2 Players and Level Picker | 1–4 Players |
|---|---|---|
| Arena | 320 × 180 px | 384 × 216 px. Sprites appear about 17% smaller at the same display size. |
| Ships at spawn | Player 1 centred, player 2 40 px to the right | Active party centred with 60 px between ships |
| Joining invincibility | 2 s | 2 s |
| Regular enemies | 8 per row | 12 per row, with the same rows and enemy types |
| Roaming heavies | By level and wave. See [Enemy Waves](../enemy-waves/design.md). | One extra per formation wave, two extra from wave 11. This includes Space. |
| Simultaneous divers | By difficulty wave | One extra from the first diving wave |
| Bosses on waves 5, 10, 15, 20 | 1, 2, 1, 2 | 2, 3, 2, 3. Each boss sways in its own lane and staggers its first attack. |
| Enemy and boss HP at wave spawn | 1× solo, 1.5× with two players | 1.5×, 1.75×, 2× and 2.25× with one through four players respectively |
| Weapon upgrade per enemy kill | 1% | 2% |
| Rocket pickup per enemy kill | 2% | 2% |
| Fisherman's live fish limit | 3 across the wave | 5 across the wave |

HP rounds up. A drop-in affects HP from the next wave. Each variant's boss types, rewards and guaranteed boss drops follow the world rules. Initial 1–4 player zoom and balance choices are subject to couch playtesting.

## Roles for three or four players

In Classic 1–4 Players, a party of three or four chooses roles before entering play. Each player chooses on their own controller and presses X to ready. Roles may repeat. If a smaller party grows to three mid-run, play pauses for everyone to choose. Once roles are active, a late fourth player chooses their own role and the existing players keep theirs. Roles persist through death and between worlds. Play Again offers another selection with the previous roles preselected.

| Role | Movement | R1 fire interval | Damage per R1 shot | R2 |
|---|---:|---:|---:|---|
| Fighter | 100% | 100% | 1 HP | Standard rocket |
| Tank | 75% | 150% | 2 HP | Heavy rocket at 350 px/s with a 60 px blast radius |
| Assassin | 130% | 70% | 0.75 HP | Phase through enemy attacks for 5 s |
| Mage | 100% | 100% | 1.5 HP | Seven piercing orbs in an upward fan aimed toward the nearest enemy above the ship |

- [ASSUMPTION] The table gives initial tuning. Each Mage orb starts at 4 HP of damage and halves its remaining damage after each enemy it hits. The orbs travel at 215 px/s with 0.14 radians between them and up to 0.4 radians of aim from vertical.
- [ASSUMPTION] The Tank's first lethal hit per life breaks its armor and gives 0.75 s of grace. The next lethal hit costs a life and armor returns on respawn. A level-buff shield absorbs the hit first when present.
- [ASSUMPTION] Role specials use the existing rocket ammo and cooldown. Square cycles between the role special and any earned active level buffs. Assassin cannot spend a second charge to extend an active phase.

## Edge cases
- Joining works during play, including the pause between waves and a teammate's respawn. The next wave starts once any ship is alive.
- Classic ends when all players are out or the Deep Sea is cleared. The Level Picker ends when all players are out or the selected level is cleared.
- A full party wipe still ends the run when it coincides with a boss-wave clear. In 1–4 Players and the Level Picker, the boss-wave clear gives the remaining players the usual score bonus.
- Divers steer toward the nearest living ship.
- The controller minimum applies to starting a 1–4 player run. Once playing, unused pads may disconnect and the joined party can keep playing.

## History
- 2026-11-17: designed and built as drop-in join (D-009).
- 2026-11-17: numbers confirmed in single-player and co-op playtests.
- 2026-11-21: Classic split into two variants with a live pad check and a larger party arena (D-055).
- 2026-11-21: added role selection and four ship roles for expanded parties of three or four (D-058).
- 2026-11-21: added boss-wave revives to Classic 1–2 Players (D-059).
