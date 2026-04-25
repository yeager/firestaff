# DM1 all-graphics phase 108 — dialog/endgame matrix after debug-label cleanup

## Problem

Phase 107 changed runtime behavior: default V1 now hides the most obvious placeholder/debug labels in the dialog/endgame overlays unless `showDebugHUD=1`:

- `TEXT PLAQUE`
- `PRESS ANY KEY TO DISMISS`
- `VICTORY AT TICK ...`
- `ESC TO RETURN TO MENU`

The matrix still described those strings as current visible overlay content without noting the default-V1 suppression.

## Change

Updated the dialog/endgame row in `PARITY_MATRIX_DM1_V1.md`:

- still records the source target: ReDMCSB `DIALOG.C:F0427_DIALOG_Draw` and separate `ENDGAME.C` flow
- records the pass-107 cleanup: placeholder/helper labels are hidden in default V1 unless `showDebugHUD=1`
- keeps status honest as `KNOWN_DIFF (narrowed/debug-cleaned)` because composition is still not the source dialog-box/endgame pipeline
- next action now focuses on wiring source dialog/endgame frontend graphics and original overlay comparison

Also refreshed the bottom line to mention the debug-label cleanup.

## Gate

Documentation-only pass, backed by phase 107:

```text
PASS INV_GV_165B V1 endgame overlay keeps tick/help text debug-only
PASS INV_GV_172B V1 dialog overlay keeps placeholder title/footer debug-only
# summary: 421/421 invariants passed
ctest: 5/5 PASS
```
