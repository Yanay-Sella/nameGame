#!/bin/bash
# Create or update an agent's worktree: scripts/agent-sync.sh claude|codex  (safe to run again)
set -e
agent=$1
case "$agent" in claude|codex) ;; *) echo "usage: $0 claude|codex"; exit 1 ;; esac
main=$(dirname "$(git rev-parse --path-format=absolute --git-common-dir)")
dir="$main-$agent"
branch="$agent/work"

if [ ! -d "$dir" ]; then
  if git -C "$main" show-ref --verify --quiet "refs/heads/$branch"; then
    git -C "$main" worktree add "$dir" "$branch"
  else
    git -C "$main" worktree add -b "$branch" "$dir" main
  fi
fi

# Gitignored files a new worktree needs (PS3 address, songs).
[ -f "$main/local.mk" ] && [ ! -f "$dir/local.mk" ] && cp "$main/local.mk" "$dir/"
mkdir -p "$dir/pkgfiles/USRDIR"
for f in "$main"/pkgfiles/USRDIR/*.ogg; do
  [ -f "$f" ] && [ ! -f "$dir/pkgfiles/USRDIR/$(basename "$f")" ] && cp "$f" "$dir/pkgfiles/USRDIR/"
done

if [ -n "$(git -C "$dir" status --porcelain)" ]; then
  echo "NOT merged: $dir has uncommitted work. Commit it first (scripts/agent-merge.sh $agent \"message\"), then run this again."
else
  git -C "$dir" merge --no-edit main
fi
echo "Worktree: $dir (branch $branch). Start $agent from that folder: cd \"$dir\""
