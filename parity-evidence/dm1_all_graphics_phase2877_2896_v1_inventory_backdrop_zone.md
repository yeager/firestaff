# DM1 all-graphics phase 2877-2896 — V1 inventory backdrop source zone

Priority lane: Inventory.

## Source-backed implementation

Added explicit helpers for the original inventory backdrop/destination:

- `M11_GameView_GetV1InventoryBackdropGraphicId()` returns source graphic `17` (`DEFS.H` `C017_GRAPHIC_INVENTORY`).
- `M11_GameView_GetV1InventoryBackdropZone()` returns the DM1 viewport replacement rectangle `(0,33,224,136)`.

This is intentionally separate from the current modern Firestaff inventory overlay so the renderer migration can target the original backdrop without relying on dialog helper names or invented full-screen panel coordinates.

## Verification added

- `INV_GV_357`: verifies source graphic id `17` and destination rectangle `(0,33,224,136)`.
- `INV_GV_358`: verifies GRAPHICS.DAT graphic `17` loads as a 224×136 viewport-sized bitmap when original assets are present.

## Result

Inventory rendering now has source-backed anchors for backdrop graphic, destination rectangle, and slot-box index/zone namespace. The remaining high-value inventory work is to migrate the visible overlay composition from Firestaff's current full-screen layout toward this C017 viewport-replacement path.
