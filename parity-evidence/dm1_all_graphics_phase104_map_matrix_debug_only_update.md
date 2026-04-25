# DM1 all-graphics phase 104 — map overlay matrix debug-only update

## Problem

Phase 103 changed runtime behavior so the invented Firestaff map overlay is ignored in default V1 parity play and only remains available when `showDebugHUD=1`. The parity matrix still described the overlay as toggled by `M` / `M12_MENU_INPUT_MAP_TOGGLE` without noting that default V1 now blocks it.

## Change

Updated the map-overlay row in `PARITY_MATRIX_DM1_V1.md`:

- source side still says ReDMCSB `NEWMAP.C` is map-transition plumbing, not player automap UI
- Firestaff side now records the pass-103 runtime behavior:
  - overlay remains present as debug surface
  - default V1 chrome mode ignores `M12_MENU_INPUT_MAP_TOGGLE` unless `showDebugHUD=1`
  - probes `INV_GV_181/181B/197` lock debug activation and default ignored behavior
- status is now `KNOWN_DIFF (debug-only)`
- next action changed from “hide/disable” to “keep debug-only unless contrary source evidence appears”

Also refreshed the bottom line to mention that the map overlay is now unreachable in normal V1 parity play.

## Gate

Documentation-only pass, backed by phase 103:

```text
PASS INV_GV_181 MAP_TOGGLE input activates debug map overlay
PASS INV_GV_181B default V1 parity input ignores invented map overlay
PASS INV_GV_197 MAP_TOGGLE closes inventory and opens debug map
# summary: 419/419 invariants passed
ctest: 5/5 PASS
```
