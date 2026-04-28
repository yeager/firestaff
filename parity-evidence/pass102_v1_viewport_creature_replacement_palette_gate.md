# Pass 102 — V1 viewport creature replacement palette gate

Date: 2026-04-28
Lane: Viewport/world visuals (DM1 V1 creatures/evidence)

## Scope

Added a focused probe for the renderer-facing DM1 creature replacement-color seam. This is not a new art path; it pins that the existing M11 creature compositor resolves the `G0243_as_Graphic559_CreatureInfo` replacement-color set nibbles into the `VIDRV_12_SetCreatureReplacementColors` slot-9/slot-10 palette targets before drawing creature sprites.

## Source/data anchors checked

- Firestaff `m11_game_view.c` creature aspect table carries `replacementColorSetIndices` from source-backed `G0243_as_Graphic559_CreatureInfo`.
- Firestaff `m11_creature_replacement_colors()` resolves those nibbles through the VIDRV-style replacement-color table used by `m11_draw_creature_sprite_ex()`.
- N2 original reference data exists at `~/.openclaw/data/firestaff-original-games/DM/_extracted/` and current runtime `GRAPHICS.DAT` is available through `$HOME/.firestaff/data`.

## What changed

- Added `INV_GV_114F2` to `probes/m11/firestaff_m11_game_view_probe.c`.
- The new gate samples:
  - Giant Scorpion: slot 9 remaps, slot 10 remains default.
  - Swamp Slime: slot 10 remaps, slot 9 remains default.
  - Giant Wasp / Red Dragon / Demon-family samples: both-slot replacement combinations.
  - Giggler: no replacement colors.
- Updated `PARITY_MATRIX_DM1_V1.md` so the stale creature-palette row no longer claims rendering integration is absent.

## Verification

```sh
cmake --build build --target firestaff_m11_game_view_probe -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
```

Observed key lines:

```text
PASS INV_GV_114F2 creature slot-9/slot-10 replacement colors match source samples
# summary: 580/580 invariants passed
```

Full local run log retained at `parity-evidence/runs/pass102_viewport_creature_palette_20260428T1120Z/output.log` (ignored run artifact).

This still does not claim pixel-perfect creature parity. The remaining blocker is the same as the wider viewport lane: run semantically matched original DM1 creature-heavy captures and overlay them against deterministic Firestaff fixtures.
