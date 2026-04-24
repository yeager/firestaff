# Pass 46 — DM1 PC 3.4 VGA palette lookup

Date: 2026-04-24
Scope: `V1_BLOCKERS.md` §10, bounded to base palette + brightness lookup.

## Goal

Retire the stale claim that Firestaff's compat palette uses EGA colors or
linear brightness attenuation.  The actual compat layer already contains the
source-backed DM1 PC 3.4 VGA values; this pass makes that state explicit and
probe-backed.

## Source grounding

- `palette-recovery/recovered_palette.json` records the VIDEODRV.C-derived
  values from ReDMCSB.
- `vga_palette_pc34_compat.c` exposes:
  - `G9010_auc_VgaPaletteBrightest_Compat[16][3]`
  - `G9010_auc_VgaPaletteAll_Compat[6][16][3]`
  - `F9010_VGA_GetColorRgb_Compat(colorIndex, paletteLevel)`

## Probe

`./run_firestaff_m11_pass46_vga_palette_probe.sh`

The probe verifies 7/7 invariants:

1. palette dimensions are 16 colors × 6 brightness levels
2. base palette index 4 is VGA cyan `(0,219,219)`, not EGA dark red
3. brown/tan/blue base colors match recovered VGA values
4. cyan is invariant across all six brightness levels
5. LIGHT5 has 8 residual non-black colors, proving it is not all-black linear attenuation
6. sampled LIGHT1–LIGHT5 values match the recovered per-level lookup tables
7. out-of-range color/level values are rejected

## What this pass does not claim

- No creature palette replacement/rendering integration.
- No special credits/entrance/swoosh palette switching.
- No viewport/layout/pixel-overlay parity claim beyond the palette lookup seam.
