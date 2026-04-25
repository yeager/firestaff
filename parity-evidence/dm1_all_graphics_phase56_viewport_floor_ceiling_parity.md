# DM1 all-graphics phase 56 — viewport floor/ceiling parity flip

## Change

Implemented ReDMCSB `F0128_DUNGEONVIEW_Draw_CPSF` parity behavior for the DM1 viewport floor/ceiling base:

```c
(mapX + mapY + direction) & 1
```

- Odd parity: ceiling normal, floor horizontally flipped.
- Even parity: ceiling horizontally flipped, floor normal.

This matches the original CPSF draw branch and prevents the GRAPHICS.DAT floor/ceiling dither from being drawn with the same orientation in every viewed square.

## Gate

Added invariant:

- `INV_GV_351` — normal V1 viewport floor/ceiling obeys DM1 parity flip

The gate renders two non-debug V1 frames with adjacent parity and verifies that the viewport floor/ceiling samples differ when GRAPHICS.DAT assets are available.

## Visual capture

Fresh capture series:

- `verification-m11/viewport-parity-clean-20260425-143212/01_ingame_start_latest.png`
- `verification-m11/viewport-parity-clean-20260425-143212/05_ingame_after_cast_latest.png`

Visual review:

- Viewport no longer reads as broken geometry or random memory garbage.
- Speckled floor/ceiling texture remains, but appears consistent with source asset dither.
- `05_ingame_after_cast_latest.png` is currently the most presentable in-game frame.
- Remaining roughness: heavy source dither/noisy viewport texture and incomplete pixel-perfect wall/object composition.

## Verification

```text
PASS INV_GV_350 normal V1 top chrome strip contains no title/debug text pixels
PASS INV_GV_351 normal V1 viewport floor/ceiling obeys DM1 parity flip
# summary: 392/392 invariants passed
ctest: 4/4 PASS
```
