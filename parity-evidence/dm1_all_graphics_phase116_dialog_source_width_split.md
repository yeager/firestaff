# DM1 all-graphics phase 116 — dialog source-width line split

## Problem

Pass 115 moved single-choice dialog message text into the reconstructed C469 vertical zone, but the long-text split still used the old procedural rule:

```text
split at nearest space before character 40
second line at +14 px
```

ReDMCSB `DIALOG.C:F0427_DIALOG_Draw` uses source font metrics and the resolved dialog zone width instead:

```c
F0645_GetStringPixelDimensions(...)
lineCount = width <= zoneWidth - (zoneWidth >> 3) ? 1 : 2;
F0646_GetLargestPrintableSubString(...,
    min(zoneWidth, width - (width >> 3)))
...
L2344_ui_Y += G2088_C7_TextLineHeight + G2085_;
```

For the current single-choice path, the zone is `C469_ZONE_DIALOG` (`112,49`..`188,73`), width 77 px.

## Change

Added source-dialog split helper:

```c
m11_dialog_source_split_two_lines(...)
```

When the source dialog backdrop is active:

- one/two-line decision is based on source-style measured text width
- split target uses C469 width minus the source 1/8 guard
- split prefers the largest space-delimited prefix that fits
- two-line vertical layout uses source line step (`G2088_C7 + G2085 = 8`), not the old 14 px procedural gap

The procedural/fallback dialog path still uses the old 40-character behavior.

## Gate

Added invariant:

- `INV_GV_172G` — V1 long dialog message uses source-width two-line split

```text
PASS INV_GV_172 Dialog overlay renders differently from normal
PASS INV_GV_172B V1 dialog overlay keeps placeholder title/footer debug-only
PASS INV_GV_172C V1 dialog overlay blits source C000 dialog-box backdrop
PASS INV_GV_172D V1 dialog overlay prints source C450 version-zone text
PASS INV_GV_172E V1 dialog message text is centered in source viewport region
PASS INV_GV_172F V1 single-choice dialog message uses reconstructed C469 vertical zone
PASS INV_GV_172G V1 long dialog message uses source-width two-line split
# summary: 426/426 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for full `F0427_DIALOG_Draw` parity:

- C471 zone path for 2/3/4-choice dialogs
- source choice text zones C462–C467
- 1/2/4-choice patch graphics
- choice input flow
