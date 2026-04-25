# DM1 all-graphics phase 119 — dialog 3/4-choice source zones

## Problem

Pass 118 gated the 2-choice path (`C471` message zone plus `C463`/`C462` choices), but source parity for the 3/4-choice zone layouts still lacked explicit proof.

ReDMCSB `DIALOG.C:F0427_DIALOG_Draw` uses:

```c
case 3:
    ZoneIndex = C471_ZONE_DIALOG;
    F0649_PrintCenteredTextToViewportZone(C463_ZONE_DIALOG_TOP_CHOICE, ... Choice1);
    F0649_PrintCenteredTextToViewportZone(C466_ZONE_DIALOG_BOTTOM_LEFT_CHOICE, ... Choice2);
    F0649_PrintCenteredTextToViewportZone(C467_ZONE_DIALOG_BOTTOM_RIGHT_CHOICE, ... Choice3);
    break;

case 4:
    ZoneIndex = C471_ZONE_DIALOG;
    ... patch 4-choice graphic ...
    F0649_PrintCenteredTextToViewportZone(C464_ZONE_DIALOG_TOP_LEFT_CHOICE, ... Choice1);
    F0649_PrintCenteredTextToViewportZone(C465_ZONE_DIALOG_TOP_RIGHT_CHOICE, ... Choice2);
    F0649_PrintCenteredTextToViewportZone(C466_ZONE_DIALOG_BOTTOM_LEFT_CHOICE, ... Choice3);
    F0649_PrintCenteredTextToViewportZone(C467_ZONE_DIALOG_BOTTOM_RIGHT_CHOICE, ... Choice4);
    break;
```

Runtime drawing already had C463–C467 placement support from pass 117; it needed explicit coverage.

## Change

Added focused gates for 3-choice and 4-choice dialog layouts:

- 3-choice: `C463`, `C466`, `C467`
- 4-choice: `C464`, `C465`, `C466`, `C467`

No runtime logic change was needed in this pass; the goal was to make the existing source-zone support non-regressable.

## Gates

Added invariants:

- `INV_GV_172J` — V1 three-choice dialog uses source C463/C466/C467 zones
- `INV_GV_172K` — V1 four-choice dialog uses source C464-C467 zones

```text
PASS INV_GV_172H V1 source dialog renders bottom C462 choice text zone
PASS INV_GV_172I V1 two-choice dialog uses source C471/C463/C462 zones
PASS INV_GV_172J V1 three-choice dialog uses source C463/C466/C467 zones
PASS INV_GV_172K V1 four-choice dialog uses source C464-C467 zones
# summary: 430/430 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for full `F0427_DIALOG_Draw` parity:

- 1/2/4-choice patch graphics (`M621`/`M622`/`M623` negative bitmaps)
- choice input flow / hit zones
- original overlay comparison captures
