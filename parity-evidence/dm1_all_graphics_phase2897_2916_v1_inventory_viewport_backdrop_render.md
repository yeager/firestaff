# DM1 all-graphics phase 2897-2916 — V1 inventory viewport backdrop render

Priority lane: Inventory.

## Source-backed implementation

Normal V1 inventory rendering now uses the source C017 inventory backdrop as a viewport replacement at `(0,33,224,136)` instead of drawing the old Firestaff full-screen inventory chrome.

The previous expansive layout is retained only behind `showDebugHUD` as a diagnostic/workbench path. Normal parity play stops after drawing C017 until dynamic object/slot rendering is migrated onto source zones `C507..C536`.

## Verification added

- `INV_GV_359`: renders normal V1 and inventory-active V1 frames, then asserts inventory changes are confined to the source viewport replacement rectangle and do not touch the right column, party/status boxes, or message area.

## Result

This removes a large invented inventory overlay surface from normal V1 and gives the next inventory passes a clean source-backed base for slot/icon composition.
