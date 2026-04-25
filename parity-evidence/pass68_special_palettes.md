# Pass 68 — Special credits/entrance VGA palette seam

Last updated: 2026-04-25
Scope: **DM1 / PC 3.4 / English / V1 original-faithful graphics** — source-backed special palette data and lookup seam only.

## Source anchor

Local ReDMCSB source:

- `/Users/bosse/.openclaw/workspace-main/ReDMCSB_WIP20210206/Toolchains/Common/Source/DRAWVIEW.C:25` — `G8147_CREDITS[17]`
- `/Users/bosse/.openclaw/workspace-main/ReDMCSB_WIP20210206/Toolchains/Common/Source/DRAWVIEW.C:43` — `G8148_ENTRANCE[17]`
- `DRAWVIEW.C:420-421` — palette table entries `G8147_CREDITS` (`6`) and `G8148_ENTRANCE` (`7`)

Rows are original VGA DAC 6-bit `COLOR_DEF` entries `{index, r, g, b}`. Firestaff stores the converted RGB8 values using the same conversion already used by the base palette seam:

```text
rgb8 = (vga6 << 2) | (vga6 >> 4)
```

## What changed

- Added `G9011_auc_VgaPaletteCredits_Compat[16][3]` from `G8147_CREDITS`.
- Added `G9012_auc_VgaPaletteEntrance_Compat[16][3]` from `G8148_ENTRANCE`.
- Added indexed table `G9013_auc_VgaPaletteSpecial_Compat[2][16][3]` and lookup helper `F9011_VGA_GetSpecialColorRgb_Compat(color, specialPalette)`.
- Kept existing base palette and six dungeon brightness tables unchanged.

## Verification

`./run_firestaff_m11_pass68_special_palette_probe.sh`

Result: `# summary: 6/6 invariants passed`

The probe verifies:

- special palette namespace exposes exactly credits and entrance;
- sampled `G8147_CREDITS` rows match converted RGB8 values;
- sampled `G8148_ENTRANCE` rows match converted RGB8 values;
- special palettes are not silently falling back to the base dungeon/title palette;
- indexed table supports caller-side palette switching without branching;
- out-of-range color/palette lookups are rejected.

## Honest limitation

This pass wires source-backed data and a renderer-facing lookup seam. It does **not** yet prove that every entrance/credits frontend call site selects the special palette at the exact original moment, and it makes no pixel-overlay claim for the entrance or credits screens.
