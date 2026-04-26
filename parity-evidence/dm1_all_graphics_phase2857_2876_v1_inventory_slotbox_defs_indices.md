# DM1 all-graphics phase 2857-2876 — V1 inventory slot-box DEFS.H indices

Priority lane: Inventory.

## Source-backed correction

Passes 2837-2856 exposed the raw layout-696 slot-zone geometry but grouped `C520` with equipment. Rechecking `dm7z-extract/Toolchains/Common/Source/DEFS.H` shows the exact runtime names:

- `C507_ZONE_SLOT_BOX_08_INVENTORY_READY_HAND` through `C519_ZONE_SLOT_BOX_20_INVENTORY_QUIVER_LINE1_1` are the non-backpack inventory/equipment slot boxes.
- `C520_ZONE_SLOT_BOX_21_INVENTORY_BACKPACK_LINE1_1` through `C536_ZONE_SLOT_BOX_37_INVENTORY_BACKPACK_LINE1_9` are backpack/carried-object slot boxes.
- The source slot-box command/index namespace is `8..37`, not a zero-based Firestaff convenience index.

This pass corrects the helper grouping and adds a source-slot-box helper keyed by the original `8..37` slot-box indices.

## Verification added/updated

- `INV_GV_353` now verifies non-backpack `C507..C519` count and geometry.
- `INV_GV_354` now verifies backpack `C520..C536` count and geometry, including first backpack slot `C520`.
- `INV_GV_355` verifies corrected bounds rejection.
- `INV_GV_356` verifies the exact source slot-box index span `8..37` maps to `C507..C536`.

## Result

Inventory overlay migration can now target the original DEFS.H slot-box namespace directly instead of a Firestaff-created grouping.
