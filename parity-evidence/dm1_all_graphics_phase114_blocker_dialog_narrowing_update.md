# DM1 all-graphics phase 114 — blocker list after dialog narrowing

## Problem

`V1_BLOCKERS.md` still described dialog/endgame overlays mostly at the pass-105 blocker level, before the pass 107–113 narrowing work.

## Change

Updated the overlay blocker section to record the current, narrower state:

- pass 107 hides invented placeholder/debug labels from default V1 unless `showDebugHUD=1`
- pass 109 blits source `C000_GRAPHIC_DIALOG_BOX` from GRAPHICS.DAT graphic `17` (`224×136`) at the viewport origin
- pass 110 prints source `V3.4` text at reconstructed `C450_ZONE_DIALOG_VERSION` coordinate `(192,40)`
- pass 112 centers message text across the source viewport region using text measurement
- blocker status remains `KNOWN_DIFF (narrowed)`, because full `F0427_DIALOG_Draw` parity is not complete

Remaining gaps are now listed more precisely:

- source dialog patch/choice zones and choice input flow
- exact C469/C471 vertical placement and source line splitting
- source endgame frontend visuals
- original dialog/endgame overlay comparison captures

## Gate

Documentation-only pass, backed by the current green runtime gates from pass 112:

```text
PASS INV_GV_172C V1 dialog overlay blits source C000 dialog-box backdrop
PASS INV_GV_172D V1 dialog overlay prints source C450 version-zone text
PASS INV_GV_172E V1 dialog message text is centered in source viewport region
# summary: 424/424 invariants passed
ctest: 5/5 PASS
```
