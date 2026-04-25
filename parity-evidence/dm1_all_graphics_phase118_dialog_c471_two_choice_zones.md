# DM1 all-graphics phase 118 — dialog C471 / two-choice zones

## Problem

Pass 117 wired the single-choice C462 bottom-choice text, but all dialog message text still used the single-choice message zone path. ReDMCSB `DIALOG.C:F0427_DIALOG_Draw` switches to `C471_ZONE_DIALOG` for 2/3/4-choice dialogs:

```c
case 2:
    ZoneIndex = C471_ZONE_DIALOG;
    ... patch 2-choice graphic ...
    F0649_PrintCenteredTextToViewportZone(C463_ZONE_DIALOG_TOP_CHOICE, ... P0837_pc_Choice1);
    F0649_PrintCenteredTextToViewportZone(C462_ZONE_DIALOG_BOTTOM_CHOICE, ... P0838_pc_Choice2);
    break;
```

Layout reconstruction:

```text
C470: type=9 parent=4 d1=188 d2=36
C471: type=0 parent=470 d1=112 d2=32
```

So the multi-choice message zone is the upper source band rather than the single-choice C469 band.

## Change

Added C471 message positioning:

```c
m11_dialog_source_c471_text_y_for_lines(...)
m11_dialog_source_message_width_for_choices(...)
```

When `dialogChoiceCount > 1`, source-dialog message text now uses the C471 vertical band. The existing source-choice helper already had the C463/C462 placements for the 2-choice path; this pass gates them together with C471.

## Gate

Added invariant:

- `INV_GV_172I` — V1 two-choice dialog uses source C471/C463/C462 zones

```text
PASS INV_GV_172 Dialog overlay renders differently from normal
PASS INV_GV_172B V1 dialog overlay keeps placeholder title/footer debug-only
PASS INV_GV_172C V1 dialog overlay blits source C000 dialog-box backdrop
PASS INV_GV_172D V1 dialog overlay prints source C450 version-zone text
PASS INV_GV_172E V1 dialog message text is centered in source viewport region
PASS INV_GV_172F V1 single-choice dialog message uses reconstructed C469 vertical zone
PASS INV_GV_172G V1 long dialog message uses source-width two-line split
PASS INV_GV_172H V1 source dialog renders bottom C462 choice text zone
PASS INV_GV_172I V1 two-choice dialog uses source C471/C463/C462 zones
# summary: 428/428 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for full `F0427_DIALOG_Draw` parity:

- 3/4-choice zone gates for C464–C467
- 1/2/4-choice patch graphics
- choice input flow
