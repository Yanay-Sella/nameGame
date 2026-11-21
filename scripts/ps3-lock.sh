#!/bin/bash
# Run a command while holding the one-PS3 lock (shared by every checkout/worktree); waits its turn, and when done quits
# the game its run started (a game already running when it took the lock, such as the user's, is left alone).
# Usage: scripts/ps3-lock.sh <command...>   e.g. scripts/ps3-lock.sh bash -c 'make install launch && make shot'
set -u
[ $# -gt 0 ] || { echo "Usage: $0 <command...>"; exit 1; }

LOCK="$HOME/.nameGame-ps3.lock"

waiting_since=$SECONDS
until mkdir "$LOCK" 2>/dev/null; do
	owner_pid=$(sed -n 1p "$LOCK/owner" 2>/dev/null)
	# A dead owner (crashed run) leaves a stale lock: drop it and retry at once.
	if [ -n "$owner_pid" ] && ! kill -0 "$owner_pid" 2>/dev/null; then
		echo "Removing stale PS3 lock from dead pid $owner_pid."
		rm -rf "$LOCK"
		continue
	fi
	echo "PS3 busy (held by $(sed -n 2p "$LOCK/owner" 2>/dev/null), running $((SECONDS - waiting_since))s of waiting); retrying in 10 s..."
	sleep 10
done

echo "$$
$(pwd) since $(date +%H:%M:%S)" > "$LOCK/owner"
# Always release the lock; leave the PS3 at the XMB so the next run's deploy isn't refused, unless a game was already
# running when the lock was taken: that one is someone's game (the run's install refuses to replace it anyway).
before=$(make -s ps3-state 2>/dev/null)
[ "$before" = running ] && echo "A game was already running on the PS3; it will be left running."
trap '[ "$before" = running ] || make -s quit >/dev/null 2>&1; rm -rf "$LOCK"' EXIT
trap 'exit 130' INT TERM

echo "PS3 lock acquired after $((SECONDS - waiting_since))s."
"$@"
