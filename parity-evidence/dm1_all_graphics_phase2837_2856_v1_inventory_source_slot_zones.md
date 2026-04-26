# DM1 all-graphics phase 2837-2856 — V1 inventory source slot zones

Priority lane: Inventory (after HUD/UI zone coverage reached pass 2836).

## Source-backed implementation

Added explicit V1 inventory source-zone helpers for the original layout-696 inventory slots:

- `M11_GameView_GetV1InventoryEquipmentSlotZone*()` exposes raw C507..C520 16x16 panel slot zones.
- `M11_GameView_GetV1InventoryBackpackSlotZone*()` exposes raw C521..C536 16x16 backpack/carried-object grid zones.

The coordinates are taken from the checked-in `zones_h_reconstruction.json` layout-696 dump (`GRAPHICS.DAT` DM1 PC 3.4 English / I34E, SHA-256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`) and preserve raw source panel coordinates instead of the current modern overlay placement.

## Verification added

`firestaff_m11_game_view_probe` now verifies:

- `INV_GV_353`: equipment slot zone count, first/last zone ids C507/C520, and first/last 16x16 geometry.
- `INV_GV_354`: backpack grid count, first/last zone ids C521/C536, and first/last 16x16 geometry.
- `INV_GV_355`: invalid ordinal rejection for both source-zone helper families.

## Result

This pass does not yet replace the modern inventory overlay layout. It pins the original source slot grid so the next inventory rendering passes can migrate actual placement/draw order without inventing geometry.

## Correction note

Follow-up phase 2857-2876 rechecked `DEFS.H` names and corrected the grouping: `C520` is `BACKPACK_LINE1_1`, so non-backpack helpers now cover `C507..C519`, backpack helpers cover `C520..C536`, and the source-index helper covers original slot-box indices `8..37`.
