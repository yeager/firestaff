# DM1 all-graphics phase 112 — centered dialog message text

## Problem

After passes 109–110, the dialog overlay used the source dialog-box backdrop and printed the source `V3.4` version-zone text, but the message body was still drawn at the old procedural-panel left inset (`dlgX + 12`).

ReDMCSB `DIALOG.C:F0427_DIALOG_Draw` centers dialog message text inside the selected dialog message zone using font metrics (`F0645_GetStringPixelDimensions`, `F0646_GetLargestPrintableSubString`, centered X calculation).

## Change

Added small text measurement / centered text helpers:

```c
m11_measure_text_pixels(...)
m11_draw_text_centered_in_rect(...)
```

When the source dialog backdrop is active, dialog message lines are now centered across the DM1 viewport width instead of using the old procedural left inset.

The procedural/fallback overlay path still uses the old left-inset behavior.

## Gate

Added invariant:

- `INV_GV_172E` — V1 dialog message text is centered in the source viewport region

```text
PASS INV_GV_172 Dialog overlay renders differently from normal
PASS INV_GV_172B V1 dialog overlay keeps placeholder title/footer debug-only
PASS INV_GV_172C V1 dialog overlay blits source C000 dialog-box backdrop
PASS INV_GV_172D V1 dialog overlay prints source C450 version-zone text
PASS INV_GV_172E V1 dialog message text is centered in source viewport region
# summary: 424/424 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for full `F0427_DIALOG_Draw` parity:

- exact C469/C471 zone vertical placement and line splitting
- 1/2/4-choice patch zones
- source choice zones/input flow
