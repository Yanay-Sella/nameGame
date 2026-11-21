#!/bin/bash
# Merge an agent's branch into main (run by the user): scripts/agent-merge.sh claude|codex "commit message"
set -e
agent=$1
message=$2
case "$agent" in claude|codex) ;; *) echo "usage: $0 claude|codex \"commit message\""; exit 1 ;; esac
main=$(dirname "$(git rev-parse --path-format=absolute --git-common-dir)")
dir="$main-$agent"
branch="$agent/work"

if [ "$(git -C "$main" branch --show-current)" != "main" ] || [ -n "$(git -C "$main" status --porcelain)" ]; then
  echo "Stop: $main must be on main with no uncommitted changes (commit or stash first)."
  exit 1
fi

if [ -n "$(git -C "$dir" status --porcelain)" ]; then
  [ -n "$message" ] || { echo "Stop: $dir has uncommitted work, give a commit message."; exit 1; }
  git -C "$dir" add -A
  git -C "$dir" commit -m "$message"
fi

git -C "$main" diff main..."$branch" --stat
git -C "$main" merge --no-edit "$branch" || { echo "Conflict: run 'git merge --abort' in $main and ask the agent to run scripts/agent-sync.sh $agent and fix it."; exit 1; }
make -C "$main" test
