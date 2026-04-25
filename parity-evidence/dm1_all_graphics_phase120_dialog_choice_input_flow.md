# DM1 all-graphics phase 120 — dialog choice input flow

## Problem

Passes 117–119 rendered source dialog choice text zones, but the overlay still behaved like a generic dismissible plaque: input dismissed the dialog without preserving which source choice was selected.

ReDMCSB command tables map dialog button zones to commands:

```text
C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1
C211_COMMAND_CLICK_ON_DIALOG_CHOICE_2
C212_COMMAND_CLICK_ON_DIALOG_CHOICE_3
C213_COMMAND_CLICK_ON_DIALOG_CHOICE_4
```

and assign:

```c
G0335_ui_SelectedDialogChoice = command - (C210 - 1);
```

## Change

Added source-style selected-choice state:

```c
dialogSelectedChoice
M11_GameView_GetDialogSelectedChoice(...)
```

Input behavior now preserves selection:

- `ACCEPT` with choices selects choice `1` and dismisses the overlay
- pointer hits inside reconstructed source button zones select the matching 1..4 choice and dismiss the overlay
- non-choice dismiss keeps selected choice at `0`
- new dialogs reset selected choice to `0`

Button hit zones are grounded in the recovered layout/source command table:

- one-choice: C456 bottom button
- two-choice: C457 top, C456 bottom
- three-choice: C457 top, C460 bottom-left, C461 bottom-right
- four-choice: C458 top-left, C459 top-right, C460 bottom-left, C461 bottom-right

## Gates

Added invariants:

- `INV_GV_172L` — V1 dialog accept selects first source choice
- `INV_GV_172M` — V1 dialog mouse hit selects source choice zone

```text
PASS INV_GV_172K V1 four-choice dialog uses source C464-C467 zones
PASS INV_GV_172L V1 dialog accept selects first source choice
PASS INV_GV_172M V1 dialog mouse hit selects source choice zone
# summary: 432/432 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for full `F0427_DIALOG_Draw` parity:

- 1/2/4-choice patch graphics (`M621`/`M622`/`M623` negative bitmaps)
- original overlay comparison captures
