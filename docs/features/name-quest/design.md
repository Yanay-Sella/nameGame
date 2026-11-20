# NAME QUEST

**Status:** in progress · **Updated:** 2026-11-20 · [GDD](../../GDD.md) · [Implementation](implementation.md) · [Testing](testing.md)

NAME QUEST is a second game on the main menu: a hard 2D side-scrolling action RPG set in the shooter's worlds. You wake beside the Hearth of Names in Namula, the last village, pick one of four classes and go out through six worlds of 20 hand-made areas each, earning names (the currency) from monsters, dropping them when you die and resting at checkpoints, with a hub you keep coming back to. It has its own code and its own three save slots; the shooter's pillar 3 ("arcade, not a campaign") applies to Classic and the level picker only (D-035).

Rules and numbers: see the work-plan spec [`spec.md`](../../work-plan/name-quest/spec.md) until phase 9 folds the final numbers in here. The plan's phases are in [`name-quest-plan.md`](../../work-plan/name-quest-plan.md).

## Rules (phase 1: the engine)
1. The main menu reads CLASSIC, LEVEL PICKER, NAME QUEST, SETTINGS. NAME QUEST opens the slot screen; there is no music in NAME QUEST (D-049).
2. The slot screen shows three slots. X on an empty slot opens class select; X on a used slot puts the hero at the last rested checkpoint. Triangle on a used slot asks `DELETE SLOT n?` (NO by default); YES deletes it. Circle goes back to the main menu.
3. Class select shows KNIGHT, ARCHER, MAGICIAN and THIEF side by side with POWER, TOUGHNESS and REACH bars; left/right picks, X confirms, Circle goes back. The slot is written at once.
4. Four intro pages are typed out; X completes a page, then turns it. After the last page the hero stands beside the hearth in Namula.
5. Controls in play: walk with the stick or D-pad, X jumps (hold for a higher jump), the Thief jumps again in the air, up/down climbs ladders, down + X drops through a one-way platform, Up at an open portal goes through it, Start pauses.
6. Areas are two screens wide or tall. Walking past an open edge, or Up at an open portal, enters the next area; the first visit shows the area's name.
7. Falling into a pit or a deadly liquid kills at once: `YOU FADED` for 2.5 s, then the hero is back at the last rested checkpoint (Namula's hearth for now) and the slot is saved.
8. RETURN TO MENU from the pause menu saves the slot. Picking the slot again later, even after a reboot, puts the hero back at the checkpoint with the same class.
9. Namula has three calm areas (the village, the cliffs, the shore); the sea on the shore is its only danger. Its portals are closed stone arches for now.

## Edge cases
- Start does nothing on the slot, class and intro screens; it pauses in play and while `YOU FADED` shows.
- A controller unplugged in play opens the pause menu, which waits for it (D-014).
- A save that can't be renamed into place is written directly (logged); a missing save file is an empty slot.

## History
- 2026-11-20: designed with the user (D-035 to D-051); phase 1 engine built.
