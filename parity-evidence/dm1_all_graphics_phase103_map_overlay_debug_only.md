# DM1 all-graphics phase 103 — default V1 ignores invented map overlay

## Problem

Phase 102 correctly classified Firestaff's fullscreen map overlay as a `KNOWN_DIFF`: it is a convenience/debug surface, not a source-backed DM1 V1 UI. But the normal V1 input path still let the player press `M` and enter that invented screen.

## Change

`M11_GameView_HandleInput(...)` now ignores `M12_MENU_INPUT_MAP_TOGGLE` in default V1 chrome mode unless `showDebugHUD` is enabled.

That keeps the overlay available for debug/probe sessions, while preventing normal parity play from entering a non-original UI surface.

## Probe changes

Updated existing map-overlay tests to mark the legacy behavior as debug-only and added a new default-path invariant:

- `INV_GV_181` — `MAP_TOGGLE` activates the map overlay when `showDebugHUD=1`
- `INV_GV_181B` — default V1 parity input ignores the invented map overlay
- `INV_GV_197` — `MAP_TOGGLE` closes inventory and opens the debug map only when `showDebugHUD=1`

## Gate

```text
PASS INV_GV_181 MAP_TOGGLE input activates debug map overlay
PASS INV_GV_181B default V1 parity input ignores invented map overlay
PASS INV_GV_197 MAP_TOGGLE closes inventory and opens debug map
# summary: 419/419 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
