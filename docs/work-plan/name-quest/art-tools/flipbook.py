"""Phase 5b step 4: turns the 16 animation sheets (build/anims/<class>_t<tier>.bmp, written by quest_tests --anims) into
one self-contained HTML flipbook: a class/tier picker, the clip at 30 fps scaled 3x, play/pause, step, and 1x/0.25x.
Standard library only. Run: python3 -I docs/work-plan/name-quest/art-tools/flipbook.py build/anims shots/anims/flipbook.html
"""
import base64
import json
import os
import struct
import sys
import zlib

CLASSES = ["knight", "archer", "magician", "thief"]
COLS, CELL_W, CELL_H, GAP = 8, 320, 180, 2   # the crop (160 x 90 dp) and its 1 dp gap, in real px
FRAMES = 72   # 144 steps, every 2nd


def read_bmp(path):
    """A 24-bit BMP (bottom-up rows) as rows of (r, g, b) tuples, top row first."""
    with open(path, "rb") as f:
        data = f.read()
    offset, = struct.unpack_from("<I", data, 10)
    w, h = struct.unpack_from("<ii", data, 18)
    bits, = struct.unpack_from("<H", data, 28)
    assert bits == 24, path + ": not a 24-bit BMP"
    stride = (w * 3 + 3) & ~3
    rows = []
    for y in range(abs(h)):
        src = (abs(h) - 1 - y) if h > 0 else y
        start = offset + src * stride
        line = data[start:start + w * 3]
        rows.append([(line[x * 3 + 2], line[x * 3 + 1], line[x * 3]) for x in range(w)])
    return rows


def png_bytes(rows):
    """The save_png logic of make_quest_art.py, written to bytes."""
    h, w = len(rows), len(rows[0])
    raw = b"".join(b"\x00" + b"".join(bytes(px) for px in row) for row in rows)
    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)) +
            chunk(b"IDAT", zlib.compress(raw, 6)) + chunk(b"IEND", b""))


def frames_of(path):
    sheet = read_bmp(path)
    out = []
    for i in range(FRAMES):
        x0, y0 = (i % COLS) * (CELL_W + GAP), (i // COLS) * (CELL_H + GAP)
        cell = [row[x0:x0 + CELL_W] for row in sheet[y0:y0 + CELL_H]]
        out.append("data:image/png;base64," + base64.b64encode(png_bytes(cell)).decode())
    return out


PAGE = """<!DOCTYPE html>
<html><head><meta charset="utf-8"><title>NAME QUEST heroes: flipbook</title>
<style>
body { background: #14121e; color: #ddd; font: 14px monospace; margin: 16px; }
img { width: %(w)dpx; height: %(h)dpx; image-rendering: pixelated; display: block; margin: 8px 0; border: 1px solid #333; }
button, select { font: 14px monospace; margin-right: 6px; }
</style></head><body>
<div>
<select id="clip"></select>
<button id="play">pause</button><button id="back">&lt; step</button><button id="next">step &gt;</button>
<button id="speed">1x</button> <span id="info"></span>
</div>
<img id="view" alt="clip">
<p>R1 taps are 0.2 s apart for the first 0.8 s. The hero stops running at 0.5 s and presses R2 at 1.4 s.</p>
<script>
const CLIPS = %(clips)s;
const pick = document.getElementById("clip"), view = document.getElementById("view"), info = document.getElementById("info");
let frame = 0, playing = true, slow = false, last = 0;
for (const name of Object.keys(CLIPS)) { const o = document.createElement("option"); o.textContent = name; pick.appendChild(o); }
function show() { const f = CLIPS[pick.value]; frame = (frame + f.length) %% f.length; view.src = f[frame];
                  info.textContent = "frame " + (frame + 1) + "/" + f.length + "  t=" + (frame * 2 / 60).toFixed(2) + " s"; }
pick.onchange = () => { frame = 0; show(); };
document.getElementById("play").onclick = e => { playing = !playing; e.target.textContent = playing ? "pause" : "play"; };
document.getElementById("back").onclick = () => { playing = false; frame--; show(); };
document.getElementById("next").onclick = () => { playing = false; frame++; show(); };
document.getElementById("speed").onclick = e => { slow = !slow; e.target.textContent = slow ? "0.25x" : "1x"; };
function tick(t) { if (playing && t - last >= 1000 / 30 / (slow ? 0.25 : 1)) { last = t; frame++; show(); } requestAnimationFrame(tick); }
show(); requestAnimationFrame(tick);
</script></body></html>
"""


def main():
    folder, out = sys.argv[1], sys.argv[2]
    clips = {}
    for cls in CLASSES:
        for tier in range(4):
            clips["%s tier %d" % (cls, tier)] = frames_of(os.path.join(folder, "%s_t%d.bmp" % (cls, tier)))
    with open(out, "w") as f:
        f.write(PAGE % dict(w=CELL_W * 3, h=CELL_H * 3, clips=json.dumps(clips)))
    print("wrote", out)


if __name__ == "__main__":
    main()
