# DM1 all-graphics phase 105 — dialog/endgame overlay classification

## Problem

`PARITY_MATRIX_DM1_V1.md` listed dialog/endgame overlays as generic `UNPROVEN` because compat stubs exist but were not visually verified. That hid an important known visual difference: the current visible Firestaff overlays are placeholders, not source DM1 dialog/endgame rendering.

## Source evidence

ReDMCSB `DIALOG.C:F0427_DIALOG_Draw`:

- expands the original dialog-box graphic into the viewport
- prints `V3.4` in `C450_ZONE_DIALOG_VERSION`
- patches 1/2/4-choice layouts with dialog patch zones
- prints choice text into source choice zones
- centers up to two message strings with source text/colour rules
- uses `F0600_DIALOG_subroutine()` to blit either the viewport dialog or redraw the viewport depending on dialog set

`ENDGAME.C` is a separate source flow and has not been wired to the current visible Firestaff overlay.

## Firestaff current state

Firestaff has:

- `dialog_frontend_pc34_compat.*`
- `endgame_frontend_pc34_compat.*`
- runtime flags / functional query APIs

But the actual visible M11 overlays in `m11_game_view.c` are invented/simple surfaces:

- `TEXT PLAQUE`
- `PRESS ANY KEY TO DISMISS`
- `QUEST COMPLETE`
- `VICTORY AT TICK ...`
- `ESC TO RETURN TO MENU`

These are functional placeholders, not DM1 V1 visual parity.

## Change

Updated the matrix row:

- status: `KNOWN_DIFF`
- next action: wire source dialog/endgame frontend graphics or hide placeholders from parity claims, then capture original dialog/endgame frames for overlay comparison

Updated summary counts:

- `KNOWN_DIFF`: `13 -> 14`
- `UNPROVEN`: `~34 -> ~33`

## Gate

Documentation/source-classification pass; no runtime code changed.
