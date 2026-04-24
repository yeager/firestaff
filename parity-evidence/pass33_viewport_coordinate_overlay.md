# Pass 33 — Viewport coordinate-locked overlay (measured)

Last updated: 2026-04-24
Scope: **DM1 / PC 3.4 / English / V1 original-faithful mode** — viewport
region only.  Side-panel is Pass 34.

This is the Pass 33 evidence file required by `PASSLIST_29_36.md` §4.33.
It produces **measured, coordinate-locked numbers** for the Firestaff
viewport rectangle and horizon/wall/floor/ceiling composition, compared
against the DEFS.H constants extracted from the local ReDMCSB dump
(`../redmcsb-output/I34E_I34M/DEFS.H`).  Where ReDMCSB pixel frames are
not yet available to overlay, the row is explicitly marked
`BLOCKED_ON_REFERENCE` rather than claimed as matched.

---

## 1. Source-faithful viewport anchors (DEFS.H)

Extracted from `redmcsb-output/I34E_I34M/DEFS.H`:

| Constant | Value | Meaning |
|---|---|---|
| `C112_BYTE_WIDTH_VIEWPORT` | 112 | Viewport byte width; 112 bytes × 2 pixels/byte = **224 pixels** (4-bpp planar) |
| `C136_HEIGHT_VIEWPORT` | 136 | Viewport pixel height |
| `C000_DERIVED_BITMAP_VIEWPORT` | 0 | Derived viewport bitmap slot (224×136 per DEFS.H comment) |
| `G8177_c_ViewportColorIndexOffset` | 0x10 | Viewport uses color indices 16–31 |
| `C007_ZONE_VIEWPORT` | 7 | ViewportZone identifier |
| `C700_ZONE_VIEWPORT_CEILING_AREA` | 700 | Ceiling row zone |
| `C701_ZONE_VIEWPORT_FLOOR_AREA` | 701 | Floor row zone |

Derived:
- Original DM1 viewport native rectangle: **224×136 pixels** (4bpp, 112 bytes per scanline).
- Screen is 320×200 VGA Mode 0x0D (see `VIDEODRV.C`).
- Horizon row: splits viewport vertically at 136 / 2 = **row 68** (within viewport).

## 2. Firestaff viewport anchors (measured)

Extracted from `m11_game_view.c` enum block near line 73:

| Constant | Value |
|---|---|
| `M11_VIEWPORT_X` | 12 |
| `M11_VIEWPORT_Y` | 24 |
| `M11_VIEWPORT_W` | 196 |
| `M11_VIEWPORT_H` | 118 |

Derived:
- Firestaff viewport rectangle on the 320×200 framebuffer:
  `(x=12, y=24, w=196, h=118)`.
- Cross-checked against captured PGM frames in `verification-m11/game-view/`:
  the dungeon viewport content starts at x=12, y=24 and extends
  196×118 pixels.  The right edge is at x=208; the bottom edge at y=142.

## 3. Measured drift against DM1 original

| Anchor | DM1 / DEFS.H | Firestaff | Delta | Status |
|---|---|---|---|---|
| Viewport width (px) | 224 | 196 | **−28** px (narrower) | `KNOWN_DIFF` |
| Viewport height (px) | 136 | 118 | **−18** px (shorter) | `KNOWN_DIFF` |
| Viewport horizon row (within viewport) | 68 | 59 (118/2) | **−9** px | `KNOWN_DIFF` (derived) |
| Viewport X/Y origin on 320×200 | not encoded in DEFS.H (hardwired in BLIT path); left column panel offset determines | (12, 24) | n/a | `BLOCKED_ON_REFERENCE` |
| Viewport total pixel area | 224×136 = 30 464 | 196×118 = 23 128 | **−7 336** px (−24.1 %) | derived |

The dimensional mismatch alone is large (24 % less pixel area), so any
per-row comparison of wall depth / floor / ceiling strips will also be
off.  The specific derived-bitmap slot (`C000_DERIVED_BITMAP_VIEWPORT`)
dimensions quoted in DEFS.H (224×136) confirm this is not an internal
viewport sub-rectangle — it is the full viewport surface.

## 4. Wall / floor / ceiling composition anchors

Firestaff does not currently expose a single "wall depth rows /
floor rows / ceiling rows" manifest; its renderer is composed across
many helpers in `m11_game_view.c`.  DEFS.H gives the zone ids
(`C700_ZONE_VIEWPORT_CEILING_AREA`, `C701_ZONE_VIEWPORT_FLOOR_AREA`)
but the exact pixel rows belong to `DRAWVIEW.C` / `DUNVIEW.C` in
ReDMCSB.  Those files exist in the local dump but have not been
pixel-walked for this pass.

| Row | Evidence source | Firestaff | Status |
|---|---|---|---|
| Ceiling top row (row 0 within viewport) | `C700_ZONE_VIEWPORT_CEILING_AREA` zone id | Renderer draws ceiling within `(M11_VIEWPORT_Y, M11_VIEWPORT_Y + M11_VIEWPORT_H/2)` region; exact top row = 24 on framebuffer | `BLOCKED_ON_REFERENCE` — zone id does not give pixel row |
| Horizon row | 68 within a 136-tall viewport | 59 within a 118-tall viewport | `KNOWN_DIFF` — scales with dimensional drift |
| Floor bottom row | `C701_ZONE_VIEWPORT_FLOOR_AREA` zone id | Bottom at y=141 on framebuffer | `BLOCKED_ON_REFERENCE` |
| Wall depth rows (perspective bands) | `DRAWVIEW.C` not pixel-walked yet | Rendered per-depth in `m11_game_view.c` at scaled dimensions | `BLOCKED_ON_REFERENCE` |

These rows cannot be promoted above `BLOCKED_ON_REFERENCE` without
either a ReDMCSB pixel capture or a pixel walk through `DRAWVIEW.C`.
That is an explicit follow-up pass for 37+.

## 5. Captured Firestaff frames available for overlay

Firestaff frames already captured under `verification-screens/` suitable
for future overlay once ReDMCSB pixel captures land:

| Scenario | Firestaff capture |
|---|---|
| Entry square (initial game view) | `01_ingame_start_latest.png` / `.ppm` |
| Party turn-right from start | `02_ingame_turn_right_latest.png` / `.ppm` |
| Forward one step | `03_ingame_move_forward_latest.png` / `.ppm` |
| Party after spell cast | `05_ingame_after_cast_latest.png` / `.ppm` |

Frames for "facing closed door", "facing open door", "facing pit",
"facing teleporter" are not yet captured.  Capturing these requires a
scripted driver run with those squares in the party's front cell.  They
are tracked here as an outstanding gap; landing them does not require
code changes, only extension of the screenshot capture harness.

## 6. Status promotion / demotion proposed for `PARITY_MATRIX_DM1_V1.md` §1

| Row | Before Pass 33 | After Pass 33 | Reason |
|---|---|---|---|
| Viewport region bounds | `UNPROVEN` | `KNOWN_DIFF` (−28×−18 px vs DM1) | DEFS.H anchors + m11_game_view.c measurement |
| Main game screen (320×200 VGA) | `UNPROVEN` | `UNPROVEN` (unchanged; 320×200 itself already matches but screen-level overlay still missing) | no new pixel overlay produced |

`PARITY_MATRIX_DM1_V1.md` §1 is updated in the same commit that lands
this file to reflect the above.

## 7. Honest scope note

This pass produces measured numbers from DEFS.H constants and from the
Firestaff viewport enum, not side-by-side pixel overlays against a
ReDMCSB-rendered viewport frame.  That pixel-overlay step is explicitly
deferred to a follow-up pass (tracked for pass 37+) because:

1. There is no ReDMCSB headless rasteriser available in this tree; the
   `redmcsb-output/I34E_I34M/` dump is source-only.
2. An original DM1 PC 3.4 VGA emulator capture is also not yet in the
   repo.
3. Promoting the Viewport row to `MATCHED` without one of those would
   violate the honesty rules in `PASSLIST_29_36.md` §0 / §4.33.7.

The measured numeric drift (−28 width, −18 height) is the concrete
output this pass contributes to V1 honesty.

**This pass alone does not complete DM1/V1 parity.**
