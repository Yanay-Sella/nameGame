---
name: name-plan
description: >
  Turns a nameGame feature or change request into a complete implementation plan at
  docs/work-plan/<slug>-plan.md, written so a Claude Sonnet session can implement it exactly,
  quickly and with few tokens. Use when the user types /name-plan, or asks for "a plan for
  Sonnet", "a work plan", or "plan this so another session can build it". Plans only; never
  implements the change itself.
argument-hint: <feature or change to plan>
---

# name-plan

You are the planner; a Claude Sonnet session is the implementer. You do the expensive work once
(reading the code, resolving every decision, working out the tricky logic) so the implementer can
work straight through the plan without exploring, guessing or redesigning. Sonnet follows written
instructions literally and precisely: it builds exactly what the plan says, and fills any gap
with its own guess, usually by reading more files or adding code nobody asked for. So every gap
you leave costs tokens and accuracy later. A good plan reads like a spec, not like advice.

The model to copy is `docs/work-plan/main-menu-plan.md`, a plan that was implemented successfully.
Read it before writing your first plan in a session; match its density, tone and structure.

Request: $ARGUMENTS

## 1. Understand the request
- If the request is empty, ask the user what to plan in one short question, then stop.
- Read what the change touches, completely, so the implementer doesn't have to:
  - `docs/GDD.md` (feature index, decisions, flow) and the feature's `docs/features/<slug>/*.md` if it exists.
  - Every `source/` file the change edits or calls into. Read them in full: you need exact names, signatures,
    constants and the control flow you are changing.
  - `tests/game_tests.cpp` and the `Makefile` `test` target when the change has testable game rules.
  - `git log --oneline -15` for recent context.
- Don't read the PDF work plan, `build/`, or the third-party skills unless the request is about them.

## 2. Settle every decision before writing
List the open questions: gameplay rules, numbers (speeds, timings, sizes, colours from the palette),
button mapping, screen layout, edge cases, what's out of scope. Then:
- Answer from the GDD, the feature docs, the code and `CLAUDE.md` whatever they already answer. Don't ask those.
- Ask the user the rest with `AskUserQuestion` (batch up to 4 per call, recommended option first with a one-line reason).
  These are design choices that belong to the user, and a plan with an unresolved choice makes the implementer guess.
- Small implementation details (a variable name, which file holds a constant) are yours to decide; don't ask.
- A new library, tool or file format: follow the user's technology rule (compare 2–3 options, let them pick).
  Remember the PS3 constraints in `CLAUDE.md`: only the libraries in the Docker image, SDL port quirks, 60 fps.

## 3. Write the plan
Save it to `docs/work-plan/<slug>-plan.md` (`<slug>` = short kebab-case feature name). Use this shape:

````markdown
# Plan: <feature in a few words>

For the implementer: follow the steps in order. Every design choice below is final, so don't redesign anything.
Read only the files each step names, and don't re-read a file after editing it. Code style: follow `CLAUDE.md` and
the existing code (plain C++14, no new libraries, short one-line comments, no helpers for one-off logic).
Commit straight to `main` when done, with this plan file in the same commit.

## What the player gets
<the behaviour as the player sees it: short bullets, a flow diagram if screens change. Exact strings, buttons, numbers.>

## <Rules section if needed, e.g. "Input rules">
<cross-cutting rules that several steps rely on>

## Step 1: <what> (`source/x.h`, `source/x.cpp`)
<exact changes: new signatures as code, new constants with values, the logic in bullets,
 code only for the subtle parts (math, state machines, hysteresis, ordering)>

## Step N: tests (`tests/game_tests.cpp`, `Makefile`)
<named test functions with what each asserts; `make test` must pass>

## Step N+1: docs
<which docs to create or update, and the exact new lines for GDD status, decisions (D-0xx), change log;
 run `python3 .claude/skills/game-docs/scripts/check_docs.py docs`>

## Step N+2: PS3 check (needs the user)
<`make ps3`, `make log` in the background, `make deploy`; what the user does on the PS3, in order;
 which log lines prove it worked; what to fix if a log line shows something else>

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- On the PS3: <one sentence of observable behaviour>.
````

Rules for the content, each because of how the implementer works:
- **Exact, not approximate.** Give real names, signatures, values and strings: `const float ROCKET_SPEED = 180.0f;`, not
  "a suitable speed". The implementer will use whatever you write, and invent whatever you don't.
- **Point with anchors, not line numbers.** Write `game.cpp › updateGame, the enemy-shot loop`. Line numbers move as soon
  as step 1 edits the file.
- **Name the files per step.** Each step's heading lists the files it edits. The implementer must read a file before
  editing it, but shouldn't open anything else; if a step calls a function from another file, put that function's
  signature in the plan so the implementer doesn't need to open the file.
- **Code only where it saves the implementer from reasoning.** New declarations, struct fields and subtle logic go in code
  blocks. Straightforward edits ("add `BTN_CIRCLE = 10` to the button enum") stay as one bullet. Don't paste whole
  functions the implementer could write from a bullet; that doubles the tokens for no gain.
- **Say what not to do** where Sonnet would otherwise be tempted: no new abstraction or scene classes, don't touch file X,
  don't rerun `make_xmb_art.py`, keep the flow in one switch. One clause each.
- **Order the steps so each depends only on earlier ones**: data and declarations first, then logic, then `main.cpp`
  wiring, then tests, docs, and the PS3 check last.
- **Keep the PS3 step.** Anything that runs on the console needs the user to launch the game; spell out exactly what they
  try and which `debugNetPrintf` lines prove it, and add those log lines in the code steps.
- **No essays.** One-line reasons, only where a reader would otherwise "fix" a deliberate choice. Don't restate
  `CLAUDE.md`; the implementer loads it automatically.

## 4. Check the plan before handing it over
Re-read the plan as the implementer would, knowing only `CLAUDE.md` and the files each step names:
- Every function, constant, field and file it references exists in the code you read, or is created by an earlier step.
- No step leaves a choice open: search it for "e.g.", "maybe", "consider", "as needed", "appropriate", "TBD", "either".
- Each code block compiles in context as far as you can tell (types, includes, names match).
- Every PS3 constraint holds: no SDL calls in files the Mac tests build, nothing the SDL port treats as a no-op, no
  per-frame full-screen clears.
- The "Done when" checks can actually be run and pass.
Fix what you find. Then stop: don't implement the plan and don't commit it.

## 5. Hand over
Tell the user, briefly: the plan's path, the decisions you took on your own (one line each), and the prompt to start it.
Give the prompt in a fenced block so it can be copied:

```
Implement docs/work-plan/<slug>-plan.md exactly as written. Read only the files each step names.
```

Mention that they should start a new session with Sonnet 5.5 selected in the model picker (or run
`claude --model claude-sonnet-5-5` in a terminal), so the implementer starts with a clean context.
