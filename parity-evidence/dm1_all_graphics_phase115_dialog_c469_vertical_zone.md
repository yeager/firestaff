# DM1 all-graphics phase 115 — dialog C469 vertical message zone

## Problem

Pass 112 centered dialog text horizontally, but its vertical position was still a simple fixed viewport offset (`M11_VIEWPORT_Y + 72`). ReDMCSB `DIALOG.C:F0427_DIALOG_Draw` chooses a source message zone before calculating message Y.

For the current single-choice/dialog-dismiss path, source selects:

```c
case 1:
    ZoneIndex = C469_ZONE_DIALOG;
```

Then it resolves the zone and centers the message block vertically inside it:

```c
F0638_GetZone(ZoneIndex, L2350_ai_XYZ);
L2346_i_ZoneWidth = M708_ZONE_WIDTH(L2350_ai_XYZ);
L2347_i_ZoneHeight = M709_ZONE_HEIGHT(L2350_ai_XYZ);
...
L2344_ui_Y = M706_ZONE_TOP(L2350_ai_XYZ)
          + ((L2347_i_ZoneHeight - (L2344_ui_Y - (G2085_ * 2))) >> 1)
          + G2083_C6_ - 1;
```

The local layout reconstruction gives:

```text
C468 parent: type=9 parent=4 d1=188 d2=73
C469:        type=0 parent=468 d1=112 d2=49
```

So the reconstructed C469 rectangle is viewport-relative `(112,49)`..`(188,73)`.

## Change

Added `m11_dialog_source_c469_text_y()` and moved default V1 source-dialog message text from the old fixed offset to the reconstructed C469 vertical band. For a single line this places text at screen Y `96` instead of the old `105`.

## Gate

Added invariant:

- `INV_GV_172F` — V1 single-choice dialog message uses reconstructed C469 vertical zone

```text
PASS INV_GV_172 Dialog overlay renders differently from normal
PASS INV_GV_172B V1 dialog overlay keeps placeholder title/footer debug-only
PASS INV_GV_172C V1 dialog overlay blits source C000 dialog-box backdrop
PASS INV_GV_172D V1 dialog overlay prints source C450 version-zone text
PASS INV_GV_172E V1 dialog message text is centered in source viewport region
PASS INV_GV_172F V1 single-choice dialog message uses reconstructed C469 vertical zone
# summary: 425/425 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for full `F0427_DIALOG_Draw` parity:

- exact two-line split via `F0646_GetLargestPrintableSubString`
- C471 zone path for 2/3/4-choice dialogs
- source choice patch zones and choice input flow
