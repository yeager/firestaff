# DM1 all-graphics phase 2937-2956 — V1 inventory body equipment icons

Priority lane: Inventory.

## Source-backed implementation

Extended the normal V1 inventory dynamic icon pass from hands to body equipment source slots:

- ready/action hands: source slot-box indices `8`/`9` (`C507`/`C508`)
- head/torso/legs/feet: `10..13` (`C509..C512`)
- neck: `18` (`C517`)

All placements use `M11_GameView_GetV1InventorySourceSlotBoxZone()` and the source object-icon resolver, still without the action-area palette remap.

## Verification added

- `INV_GV_361`: renders normal V1 inventory with and without a head armour item and asserts the pixel delta is confined to source slot `C509` at screen rectangle `(34,59)-(49,74)`.

## Result

Normal V1 inventory now has source C017 static backdrop plus dynamic source-zone placement for hands and core worn-equipment slots. Pouches/quivers/backpack remain the next bounded inventory migration.
