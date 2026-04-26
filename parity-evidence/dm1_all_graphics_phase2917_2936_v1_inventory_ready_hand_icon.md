# DM1 all-graphics phase 2917-2936 — V1 inventory ready-hand icon on source slot C507

Priority lane: Inventory.

## Source-backed implementation

Normal V1 inventory now draws the active champion's ready/action hand object icons on top of the source C017 inventory backdrop using original slot-box indices:

- source slot-box `8` / `C507_ZONE_SLOT_BOX_08_INVENTORY_READY_HAND`
- source slot-box `9` / `C508_ZONE_SLOT_BOX_09_INVENTORY_ACTION_HAND`

Icons use the existing source object-icon resolver and draw without the action-area color-12-to-cyan remap. Other dynamic inventory slots remain deferred until their C507..C536 slot bindings are migrated.

## Verification added

- `INV_GV_360`: renders normal V1 inventory with and without a dagger in the ready hand, then asserts the pixel delta is confined to C507's screen rectangle `(6,86)-(21,101)`.

## Result

The normal V1 inventory path now has source C017 backdrop plus first dynamic object-icon placement on the original ready/action hand slot zones, without reviving the old full-screen overlay.
