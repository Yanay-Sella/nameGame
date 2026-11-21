#!/bin/bash
# Show whether main and the agents' branches are in sync: scripts/agent-status.sh
main=$(dirname "$(git rev-parse --path-format=absolute --git-common-dir)")
printf "%-8s %-28s %s\n" folder branch "uncommitted | ahead of main | behind main"
for pair in "main:$main:main" "claude:$main-claude:claude/work" "codex:$main-codex:codex/work"; do
  IFS=: read name dir branch <<< "$pair"
  [ -d "$dir" ] || { printf "%-8s missing (run scripts/agent-sync.sh %s)\n" "$name" "$name"; continue; }
  dirty=$(git -C "$dir" status --porcelain | wc -l | tr -d ' ')
  read behind ahead <<< "$(git -C "$main" rev-list --left-right --count main..."$branch")"
  printf "%-8s %-28s %s files | %s commits | %s commits\n" "$name" "$branch" "$dirty" "$ahead" "$behind"
done
