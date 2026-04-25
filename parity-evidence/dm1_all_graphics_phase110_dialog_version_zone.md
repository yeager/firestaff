# DM1 all-graphics phase 110 — dialog C450 version-zone text

## Problem

Pass 109 blitted the source `C000_GRAPHIC_DIALOG_BOX` backdrop, but the next source operation in ReDMCSB `DIALOG.C:F0427_DIALOG_Draw` was still missing:

```c
F0040_TEXT_Print(..., C450_ZONE_DIALOG_VERSION,
                 C02_COLOR_LIGHT_GRAY, C01_COLOR_DARK_GRAY, "V3.4");
```

## Source binding

`zones_h_reconstruction.json` gives:

```text
C450_ZONE_DIALOG_VERSION = 450
record 450: type=4, parent=4, d1=192, d2=7
```

Parent zone 4 is the 224×136 viewport, so the screen-space origin is:

```text
x = M11_VIEWPORT_X + 192 = 192
y = M11_VIEWPORT_Y + 7   = 40
```

## Change

When the source dialog backdrop is drawn, Firestaff now also prints `V3.4` at the reconstructed C450 version-zone origin using the original font path when available.

## Gate

Added invariant:

- `INV_GV_172D` — V1 dialog overlay prints source C450 version-zone text

```text
PASS INV_GV_172 Dialog overlay renders differently from normal
PASS INV_GV_172B V1 dialog overlay keeps placeholder title/footer debug-only
PASS INV_GV_172C V1 dialog overlay blits source C000 dialog-box backdrop
PASS INV_GV_172D V1 dialog overlay prints source C450 version-zone text
# summary: 423/423 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for full `F0427_DIALOG_Draw` parity:

- 1/2/4-choice patch zones
- source choice zones/input flow
- message centering/wrapping via source text metrics
