# Co-op

**Status:** in progress · **Updated:** 2026-11-17 · [GDD](../../GDD.md)

A friend can pick up a second controller and drop into a run already going, arcade style. Runs still start with one player (D-009), so nothing changes for someone playing alone. It serves pillar 3 (arcade): no lobby or menu, just press Start.

## Rules
1. A run starts with one player. While it's running and not paused, Start or X on the second controller adds player 2.
2. Player 2 joins with a fresh run of their own (3 lives, 3 rockets, 0 points) and arrives blinking, as after a respawn, 40 px right of the centre.
3. Each player has their own lives, rockets, score and extra lives. Bullet and rocket kills score for the player who fired. Pickups go to whoever touches them.
4. A player with no lives left is out until the run ends. The run ends when every player is out.
5. The wave clear bonus goes to every player still in the run.
6. Either player's Start pauses. At game over, X or Start from either player starts a new run with the same number of players.
7. HUD: player 1's score, spare lives and rockets on the left, player 2's on the right, the wave in the centre. Scores read "1P"/"2P" in co-op and "SCORE" alone. With one player, "2P PRESS START" shows dimly at the top right.
8. Player 2's ship has the same shape with an orange cockpit and blue stripes.

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Maximum players | 2 | players | |
| Player 2 spawn offset | 40 | px right of centre | ships don't overlap at the start [ASSUMPTION] |
| Join invincibility | 2 | s | same as the respawn invincibility |

## Edge cases
- Joining during the pause between waves or while player 1 is respawning: allowed. The next wave starts once any ship is alive.
- Joining while paused or at game over: ignored.
- A player who is out can't rejoin until the next run (no continues).
- Divers steer toward the nearest living ship, so with two ships they pick whichever is closer.
- Player 2's controller unplugged: their ship stays and drifts idle. Unplug handling is M4.

## History
- 2026-11-17: designed and built as drop-in join (D-009).
