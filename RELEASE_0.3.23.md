## Firestaff v0.3.23

### Bug Fix
- **Fix phantom alcove item sprites in viewport.** WALL-square items (champion alcove armour/weapons) no longer render as floor sprites in the 3D dungeon viewport at game start (Hall of Champions).
- Root cause: `m11_sample_viewport_cell` extracted floor items and creature groups from all square types including WALL squares. These alcove items were drawn as phantom floor sprites.
- Fix: Gate item and creature group extraction on `elementType != DUNGEON_ELEMENT_WALL`.
- Reference: ReDMCSB DUNVIEW.C F0115:4920 sub-cell/alcove gate.
