# DM1 all-graphics phase 2957-2976 — V1 inventory dynamic remaining slot icons

Priority lane: Inventory.

## Source-backed implementation

Normal V1 inventory now draws Firestaff-modeled pouch/quiver/backpack objects through the same source slot-box namespace as the earlier hand/body passes:

- pouch/quiver source slots: `14..17`, `19..20` → `C513..C516`, `C518..C519`
- compact Firestaff backpack slots `1..8` → source backpack slot-box indices `21..28` → `C520..C527`

The bridge is explicit in `M11_GameView_GetV1InventorySourceSlotBoxForChampionSlot()`. It is backed by the extracted source convention where inventory slot-box ids are `C08_SLOT_BOX_INVENTORY_FIRST_SLOT + SlotIndex` and the layout-696 table names `C507..C536` preserve source slot-box indices `8..37`.

## Verification added

- `INV_GV_362`: asserts the compact Firestaff champion inventory constants map onto the source slot-box indices for pouch/quiver/backpack placement, with the unused action-hand alias rejected.
- `INV_GV_363`: renders normal V1 inventory before/after placing dagger objects in `POUCH_2`, `QUIVER_3`, and `BACKPACK_8`, then asserts all pixel deltas are confined to source boxes `C513`, `C514`, and `C527`.

## Remaining gap

Firestaff's current champion compatibility model exposes eight backpack slots, so runtime dynamic drawing is covered through `C527`. Source geometry helpers still expose the full original `C520..C536` backpack namespace; dynamic object population for `C528..C536` needs a future compatibility-model expansion before those boxes can be filled from real champion state.

## Result

The normal V1 inventory overlay now has source-zone dynamic object placement for hands, body equipment, neck, pouches, quiver, and the currently modeled backpack slots without falling back to the old freehand/debug inventory layout.
