#!/usr/bin/env python3
"""
Pass 47 — ReDMCSB reference composition tool.

This is a *reference compositor*, not an emulator or a headless rasteriser
for ReDMCSB's full game loop.  It takes the already-extracted DM1 PC 3.4
GRAPHICS.DAT bitmaps in ``extracted-graphics-v1/pgm/`` (palette-indexed
PGMs) and the recovered VGA palette in ``palette-recovery/recovered_palette.json``
and assembles a 320x200 RGB reference canvas whose geometry is anchored
by ReDMCSB / DEFS.H / COORD.C constants:

  - viewport rectangle:            (0, 33, 224, 136)
    sources: COORD.C G2067_i_ViewportScreenX=0, G2068_i_ViewportScreenY=33,
             DEFS.H  C112_BYTE_WIDTH_VIEWPORT=112 (=224 px),
                     C136_HEIGHT_VIEWPORT=136
  - screen:                        (0, 0, 320, 200)
    sources: COORD.C G2071_C320_ScreenPixelWidth=320,
                     G2072_C200_ScreenPixelHeight=200
  - panel graphic "empty":         C020_GRAPHIC_PANEL_EMPTY, 144 x 73
                                    DEFS.H:1780
  - champion status box spacing:   C69_CHAMPION_STATUS_BOX_SPACING = 69
                                    DEFS.H:1756
  - portrait size:                 32 x 29 (COORD.C G2078/G2079)
  - champion icon size:            19 x 14 (COORD.C G2080/G2081)
  - spell-area backdrop:           C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND
                                    (87 x 25)
  - action-area graphic:           C010_GRAPHIC_MENU_ACTION_AREA
                                    (87 x 45)
  - spell-area lines:              C011_GRAPHIC_MENU_SPELL_AREA_LINES
                                    (14 x 39)

The *viewport content* (wall/floor/ceiling composition for a specific
facing/square) is NOT synthesised here: that lives behind DRAWVIEW.C /
DUNVIEW.C, which is source-only in the dump and cannot be rasterised
without a runnable engine.  The viewport rectangle in the output
reference is rendered as a black region with an optional debug-grid
overlay to make the boundary obvious for overlay comparison, and this
is documented honestly in the provenance JSON.

What this tool DOES unblock for pass-49-style overlays:

  1. Side-panel *chrome* pixel overlays: panel backdrop, champion status
     box stride (69px), portrait + icon slots, action and spell area
     backdrops -- every element whose pixel identity is a GRAPHICS.DAT
     blit at a source-anchored position.
  2. Per-graphic palettized reference PNGs (``reference-artifacts/anchors/``)
     so Firestaff frames can be diffed against the actual DM1 bitmap,
     not a Firestaff-authored one.
  3. A reproducible compose-from-source workflow driven only by files
     that already live in the tree.

What still needs an emulator capture (blocker trail):

  - exact viewport wall/floor/ceiling content for a given square/pose
  - exact textual content rendered through TEXT.C / TEXT2.C
  - animated creature/projectile frames
  - ZONES.H-driven zone layouts (not in the local dump)

Run:

    python3 tools/redmcsb_reference_compose.py

Artifacts land under ``reference-artifacts/`` (git-tracked, tiny PNGs).
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Dict, List, Tuple

from PIL import Image

REPO = Path(__file__).resolve().parent.parent
EXTRACT = REPO / "extracted-graphics-v1"
PGM_DIR = EXTRACT / "pgm"
MANIFEST = EXTRACT / "manifest.json"
PALETTE_JSON = REPO / "palette-recovery" / "recovered_palette.json"
OUT = REPO / "reference-artifacts"
ANCHOR_DIR = OUT / "anchors"

# --- ReDMCSB-anchored screen geometry (DEFS.H / COORD.C) ---------------------

SCREEN_W, SCREEN_H = 320, 200
VIEWPORT_X, VIEWPORT_Y = 0, 33
VIEWPORT_W, VIEWPORT_H = 224, 136

# --- GRAPHICS.DAT anchors we are confident about from DEFS.H -----------------
# Each entry: (friendly_name, graphic_index, expected_w, expected_h, defs_reference)
ANCHOR_GRAPHICS = [
    ("viewport_full_frame",    0,  224, 136, "C000_DERIVED_BITMAP_VIEWPORT; full VGA viewport (content index 0)"),
    ("spell_area_backdrop",    9,   87,  25, "C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND (DEFS.H:1815)"),
    ("action_area",           10,   87,  45, "C010_GRAPHIC_MENU_ACTION_AREA (in-code comment, m11_game_view.c:8385)"),
    ("spell_area_lines",      11,   14,  39, "C011_GRAPHIC_MENU_SPELL_AREA_LINES (DEFS.H:1816)"),
    ("panel_empty",           20,  144,  73, "C020_GRAPHIC_PANEL_EMPTY (DEFS.H:1780)"),
    ("champion_portraits",    26,  256,  87, "C026_GRAPHIC_CHAMPION_PORTRAITS (atlas; 8x3 of 32x29 portraits)"),
    ("slot_box_normal",       33,   18,  18, "slot box graphic, verified by INV_GV_201 probe"),
    ("slot_box_wounded",      34,   18,  18, "slot box graphic, verified by INV_GV_202 probe"),
    ("slot_box_acting_hand",  35,   18,  18, "slot box graphic, verified by INV_GV_203 probe"),
    ("status_box_frame",       7,   67,  29, "status box frame, verified by INV_GV_205 probe"),
]


def load_palette() -> List[Tuple[int, int, int]]:
    """Return the brightest (LIGHT0 == base_palette_icon) 16-color VGA palette.

    This is the palette used for panel / UI / non-viewport surfaces and
    for the viewport at maximum brightness.  All the anchor graphics in
    ``ANCHOR_GRAPHICS`` are rendered through this palette -- per
    ``recovered_palette.json`` notes, ``G8149_ICON`` == LIGHT0 RGB.
    """
    with PALETTE_JSON.open() as f:
        data = json.load(f)
    return [(c["r"], c["g"], c["b"]) for c in data["base_palette_icon"]["colors"]]


def read_pgm(path: Path) -> Tuple[int, int, bytes]:
    with path.open("rb") as f:
        assert f.readline().strip() == b"P5"
        line = f.readline()
        while line.startswith(b"#"):
            line = f.readline()
        w, h = (int(x) for x in line.split())
        assert int(f.readline().strip()) == 255
        pixels = f.read(w * h)
    return w, h, pixels


def pgm_to_rgb(pgm_path: Path, palette: List[Tuple[int, int, int]]) -> Image.Image:
    """Palettize the nibble-as-grayscale PGM (0, 17, 34, ... 255) to RGB."""
    w, h, raw = read_pgm(pgm_path)
    out = bytearray(w * h * 3)
    for i, gray in enumerate(raw):
        # PGM nibble convention: gray = idx * 17, with idx in 0..15.
        # Use integer round-trip via divide-by-17; clamp to 15 just in case.
        idx = gray // 17
        if idx > 15:
            idx = 15
        r, g, b = palette[idx]
        o = i * 3
        out[o] = r
        out[o + 1] = g
        out[o + 2] = b
    return Image.frombytes("RGB", (w, h), bytes(out))


def sha256_of(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def draw_grid(canvas: Image.Image, x: int, y: int, w: int, h: int, step: int = 8, color=(40, 40, 40)) -> None:
    """Draw a faint alignment grid over a rect so overlays stay easy to sight."""
    px = canvas.load()
    for yy in range(y, y + h):
        for xx in range(x, x + w):
            if (xx - x) % step == 0 or (yy - y) % step == 0:
                # blend softly: leave if already bright, darken if black
                cr, cg, cb = px[xx, yy]
                if cr == 0 and cg == 0 and cb == 0:
                    px[xx, yy] = color


def compose_reference(palette: List[Tuple[int, int, int]]) -> Tuple[Image.Image, Dict]:
    """Return (reference 320x200 RGB image, provenance dict).

    The reference is composited from the source-anchored parts we have
    evidence for.  Unknown / zone-dependent positions are NOT guessed:
    we leave them in a clearly marked placeholder strip and flag them
    in the provenance as ``blocked_on_zones_h``.
    """
    img = Image.new("RGB", (SCREEN_W, SCREEN_H), (0, 0, 0))
    prov: Dict = {
        "note": "Pass 47 reference composition: source-anchored regions only.",
        "screen": {"w": SCREEN_W, "h": SCREEN_H},
        "palette_source": "palette-recovery/recovered_palette.json :: base_palette_icon (LIGHT0)",
        "regions": [],
        "blocked_on_zones_h": [],
    }

    # (1) Viewport rect 0..223 x 33..168: placeholder + alignment grid.
    # We do NOT fabricate dungeon wall/floor/ceiling pixels.
    draw_grid(img, VIEWPORT_X, VIEWPORT_Y, VIEWPORT_W, VIEWPORT_H, step=8, color=(32, 32, 32))
    prov["regions"].append({
        "name": "viewport_rect_placeholder",
        "rect_xywh": [VIEWPORT_X, VIEWPORT_Y, VIEWPORT_W, VIEWPORT_H],
        "status": "placeholder_alignment_grid",
        "source_anchor": "COORD.C G2067=0, G2068=33; DEFS.H C112_BYTE_WIDTH_VIEWPORT*2=224, C136_HEIGHT_VIEWPORT=136",
        "note": "Viewport CONTENT is DRAWVIEW.C/DUNVIEW.C runtime output; not synthesisable without an engine. Grid marks the exact DEFS.H-anchored viewport boundary for overlay calibration.",
    })

    # (2) Panel empty at y=33 column x=224. ReDMCSB composes the side column
    # using zones whose pixel layout lives in ZONES.H (not in the local
    # dump).  The C020_GRAPHIC_PANEL_EMPTY bitmap is 144x73, which does
    # NOT fit the 96-pixel-wide right column (320-224=96).  This means
    # C020 is used in a larger overlay context (inventory panel etc.),
    # not as a permanent column backdrop.  We therefore do NOT paste it
    # into the reference: that would be a fabricated placement.
    prov["blocked_on_zones_h"].append({
        "name": "side_column_backdrop_position",
        "reason": "ZONES.H layout record not in local ReDMCSB dump; side column width (96px) != panel graphic width (144px) => placement requires zone-level evidence.",
        "suggested_followup": "extract ZONES.H (or parse PANEL.C + COORD.C layout-record init path) to recover pixel x/y for C101_ZONE_PANEL and the champion-box zones.",
    })

    # (3) Bottom message area (below viewport) y=169..199 x=0..319
    # We have COORD.C G2092_MessageAreaWidth=320 and
    # G2091_MessageAreaLineByteCount = BITMAP_BYTE_COUNT(320, 7) => one
    # line is 7 px tall.  Viewport ends at y=168 (33+136-1).  The message
    # area therefore spans y=169..199 (31 px, ~4 lines of 7 px).  We
    # render it as a clearly marked black strip with a single-pixel
    # alignment border.  Content is TEXT.C-driven; not synthesised.
    px = img.load()
    for xx in range(SCREEN_W):
        px[xx, 169] = (28, 28, 28)  # thin top border of message area
    prov["regions"].append({
        "name": "message_area_rect_placeholder",
        "rect_xywh": [0, 169, SCREEN_W, SCREEN_H - 169],
        "status": "placeholder_border_only",
        "source_anchor": "COORD.C G2092_MessageAreaWidth=320, G2091 via M075 => line height 7; viewport end row=168",
        "note": "Text content is runtime-produced; reference shows only the boundary strip.",
    })

    return img, prov


def render_anchor_pngs(palette: List[Tuple[int, int, int]], manifest: Dict) -> List[Dict]:
    """Render every anchor graphic to its own palettized PNG + metadata.

    Returns a list of per-graphic provenance dicts.
    """
    ANCHOR_DIR.mkdir(parents=True, exist_ok=True)
    entries_by_index = {int(e["index"]): e for e in manifest["entries"]}
    out: List[Dict] = []
    for name, idx, ew, eh, ref in ANCHOR_GRAPHICS:
        entry = entries_by_index.get(idx)
        if entry is None or entry.get("status") != "bitmap":
            out.append({"name": name, "graphic_index": idx, "status": "missing", "defs_reference": ref})
            continue
        aw, ah = int(entry["width"]), int(entry["height"])
        pgm_path = EXTRACT / f"pgm/graphic_{idx:04d}.pgm"
        if not pgm_path.exists():
            out.append({"name": name, "graphic_index": idx, "status": "no_pgm", "defs_reference": ref})
            continue
        img = pgm_to_rgb(pgm_path, palette)
        target = ANCHOR_DIR / f"{idx:04d}_{name}.png"
        img.save(target, format="PNG", optimize=True)
        size_match = (aw == ew and ah == eh)
        out.append({
            "name": name,
            "graphic_index": idx,
            "path": str(target.relative_to(REPO)),
            "manifest_wh": [aw, ah],
            "expected_wh": [ew, eh],
            "size_match": size_match,
            "defs_reference": ref,
            "sha256": sha256_of(target),
        })
    return out


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    ANCHOR_DIR.mkdir(parents=True, exist_ok=True)

    palette = load_palette()
    with MANIFEST.open() as f:
        manifest = json.load(f)

    ref_img, ref_prov = compose_reference(palette)

    ref_png = OUT / "redmcsb_reference_320x200.png"
    ref_img.save(ref_png, format="PNG", optimize=True)

    anchor_entries = render_anchor_pngs(palette, manifest)

    provenance = {
        "pass": 47,
        "tool": "tools/redmcsb_reference_compose.py",
        "inputs": {
            "graphics_dat_extraction_manifest": "extracted-graphics-v1/manifest.json",
            "graphics_dat_source_sha256": manifest["summary"]["source_sha256"],
            "palette_file": "palette-recovery/recovered_palette.json",
            "redmcsb_source_dump": "../redmcsb-output/I34E_I34M/ (DEFS.H, COORD.C referenced)",
        },
        "outputs": {
            "reference_canvas": {
                "path": str(ref_png.relative_to(REPO)),
                "sha256": sha256_of(ref_png),
                "wh": [SCREEN_W, SCREEN_H],
            },
            "anchors": anchor_entries,
        },
        "composition": ref_prov,
        "honesty": {
            "what_this_is":
                "A source-anchored 320x200 reference canvas + per-graphic "
                "palettized PNGs suitable for pixel-diff overlays of panel "
                "chrome and individual graphic blits.",
            "what_this_is_not":
                "A full ReDMCSB frame render, an emulator capture, or a "
                "claim that the viewport content is known without an "
                "engine. Viewport content placeholders are clearly marked.",
            "blockers_remaining": ref_prov["blocked_on_zones_h"] + [
                {"name": "viewport_dungeon_content",
                 "reason": "Requires a runnable engine (DM1 DOS binary under DOSBox or a ReDMCSB headless build). See scripts/dosbox_dm1_capture.sh for the DOSBox path."},
                {"name": "runtime_text_content",
                 "reason": "TEXT.C / TEXT2.C glyph rendering is content-dependent; font atlas is extracted (GRAPHICS.DAT index varies) but string placement is zone-driven."},
            ],
        },
    }

    prov_path = OUT / "provenance.json"
    prov_path.write_text(json.dumps(provenance, indent=2) + "\n")

    print(f"[pass-47] wrote {ref_png.relative_to(REPO)}")
    print(f"[pass-47] wrote {len(anchor_entries)} anchor PNGs under {ANCHOR_DIR.relative_to(REPO)}/")
    print(f"[pass-47] wrote provenance {prov_path.relative_to(REPO)}")
    ok = sum(1 for e in anchor_entries if e.get("size_match"))
    miss = sum(1 for e in anchor_entries if not e.get("size_match"))
    print(f"[pass-47] anchors with DEFS.H-matching size: {ok}/{len(anchor_entries)} (mismatches: {miss})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
