"""Copies the sprites in new-sprites.h into source/sprites.h (small-fixes-plan.md, step 1).

Run from the repo root: python3 docs/work-plan/small-fixes/apply_sprites.py
A sprite that already exists is replaced in place; a new one is added just above the font.
"""
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
TARGET = os.path.join(HERE, "..", "..", "..", "source", "sprites.h")
SPRITE = r"const Sprite %s = \{\d+, \d+,(?:\s*\"[^\"]*\")+\};"

new = open(os.path.join(HERE, "new-sprites.h")).read()
text = open(TARGET).read()
for match in re.finditer(SPRITE % r"(\w+)", new):
    name, definition = match.group(1), match.group(0)
    rows = re.findall(r'"([^"]*)"', definition)
    w, h = map(int, re.match(r"const Sprite \w+ = \{(\d+), (\d+),", definition).groups())
    assert len(rows) == h and all(len(r) == w for r in rows), name
    if re.search(SPRITE % name, text):
        text = re.sub(SPRITE % name, lambda m: definition, text)
        print("replaced", name)
    else:
        text = text.replace("// 3x5 font", definition + "\n\n// 3x5 font", 1)
        print("added", name)
open(TARGET, "w").write(text)
