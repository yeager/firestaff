# Candidate pass 2897–2902 — Inventory/Viewport source-seam probe batch

Branch: `parallel/inventory-viewport-20260426085439`

## Scope

Non-invasive Inventory + Viewport evidence only.  This candidate pass avoids the HUD/UI rendering rewrite area and does not change runtime drawing code.

## Inventory evidence locked

Added `firestaff_m11_game_view_probe` invariants `INV_GV_409`–`INV_GV_412`:

- inventory panel seam: GRAPHICS.DAT graphic `C020` via layout-696 `C101_ZONE_PANEL`, expected zone `(80,52,144,73)`.
- slot-box graphics: `C033/C034/C035`, 18×18 source boxes with the known 16×16 icon-cell overhang.
- object-icon atlas: source 16×16 cells, 32 icons per graphics page, starting at graphic `42`.
- action-vs-inventory palette split: action icons apply the `G0498` colour-12→cyan remap, inventory slot icons preserve source colour 12.

This strengthens the existing narrowed inventory row without claiming full inventory layout parity.  The current full inventory overlay still needs side-by-side original capture/overlay before the matrix row can retire.

## Viewport evidence locked

Added `firestaff_m11_game_view_probe` invariants `INV_GV_408` and `INV_GV_413`:

- viewport zone `C007` is still pinned to DM1 source rect `(0,33,224,136)`.
- viewport content placement seams expose source-backed C2500 object, C3200 creature, and C2900 projectile points inside C007.

This is structural placement evidence, not a pixel overlay claim.  Remaining viewport work is still draw-order/content overlay against original runtime captures.

## Gate

`firestaff_m11_game_view_probe`: `532/532 invariants passed` after adding the six candidate assertions.
