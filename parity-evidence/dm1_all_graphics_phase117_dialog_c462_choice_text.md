# DM1 all-graphics phase 117 — dialog bottom choice text zone C462

## Problem

The source dialog renderer always draws choice text after message text. For a one-choice dialog, ReDMCSB `DIALOG.C:F0427_DIALOG_Draw` does:

```c
case 1:
    ZoneIndex = C469_ZONE_DIALOG;
    ... patch 1-choice graphic ...
    F0649_PrintCenteredTextToViewportZone(
        C462_ZONE_DIALOG_BOTTOM_CHOICE,
        C15_COLOR_WHITE,
        C05_COLOR_LIGHT_BROWN,
        P0837_pc_Choice1);
    break;
```

Firestaff's source dialog backdrop/message path had no visible choice text; it still behaved like a generic dismissible plaque.

## Change

Added source-style dialog choice state:

```c
dialogChoiceCount
dialogChoices[4][32]
M11_GameView_ShowDialogOverlayChoices(...)
```

`M11_GameView_ShowDialogOverlay(...)` now defaults to one source-style choice label (`OK`) instead of a choice-less plaque.

Added source-choice text drawing helper for C462–C467-style placements. This pass wires the single-choice bottom zone path:

- `C462_ZONE_DIALOG_BOTTOM_CHOICE`
- parent source record path: C462 -> C456 -> C454 -> viewport
- visible text centered in the lower choice band

The old "press any key" debug hint remains hidden from default V1 unless debug HUD is enabled.

## Gate

Added invariant:

- `INV_GV_172H` — V1 source dialog renders bottom C462 choice text zone

```text
PASS INV_GV_172 Dialog overlay renders differently from normal
PASS INV_GV_172B V1 dialog overlay keeps placeholder title/footer debug-only
PASS INV_GV_172C V1 dialog overlay blits source C000 dialog-box backdrop
PASS INV_GV_172D V1 dialog overlay prints source C450 version-zone text
PASS INV_GV_172E V1 dialog message text is centered in source viewport region
PASS INV_GV_172F V1 single-choice dialog message uses reconstructed C469 vertical zone
PASS INV_GV_172G V1 long dialog message uses source-width two-line split
PASS INV_GV_172H V1 source dialog renders bottom C462 choice text zone
# summary: 427/427 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for full `F0427_DIALOG_Draw` parity:

- C471 path for 2/3/4-choice dialogs
- remaining choice text zones C463–C467
- 1/2/4-choice patch graphics
- choice input flow
