# DM1 all-graphics phase 106 — overlay blocker addendum

## Problem

Passes 102–105 classified two important non-original Firestaff surfaces in the parity matrix:

- map overlay: convenience/debug overlay, now debug-only
- dialog/endgame overlays: functional placeholders, not source DM1 visual parity

`V1_BLOCKERS.md` did not yet have a durable summary of that classification.

## Change

Added an all-graphics overlay addendum to `V1_BLOCKERS.md`:

- records that ReDMCSB `NEWMAP.C` is map-transition plumbing, not an automap UI
- records Firestaff map overlay as `KNOWN_DIFF (debug-only)`
- records pass-103 runtime behavior: default V1 ignores `MAP_TOGGLE` unless `showDebugHUD=1`
- records probe gates `INV_GV_181`, `INV_GV_181B`, `INV_GV_197`
- records ReDMCSB `DIALOG.C:F0427_DIALOG_Draw` source behavior
- records Firestaff `TEXT PLAQUE` / `QUEST COMPLETE` overlays as placeholders / `KNOWN_DIFF`
- lists remaining gaps: source dialog/endgame visuals, original frame capture/overlay, keeping map debug-only

## Gate

Documentation-only pass, backed by recent green state:

```text
firestaff_m11_game_view_probe: 419/419 invariants passed
ctest: 5/5 PASS
```
