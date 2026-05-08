# CSB V1 inventory render/grid source-lock pass — 2026-05-08

Result: PASS — source evidence and verifier for a narrow CSB V1 inventory render/grid/viewport seam. This is not a rendered/pixel parity claim and does not enable CSB launch support.

## Scope

- Inventory screen rendering: `ShowHideInventory` opens inventory mode, draws backdrop `17` into `d.pViewportBMP`, labels HEALTH/STAMINA/MANA, draws 30 backpack/body slots, flags viewport/portrait/chest/possession/stat/cursor redraw, and switches to inventory button routing.
- Item rendering: `DrawItem` and `GetIconGraphic` resolve icon atlas groups through `d.Word612`, draw 16x16 icon rectangles from graphics `42+group`, and choose `d.pViewportBMP`/width `112` for inventory/backpack slots versus `d.LogicalScreenBase`/width `160` for top-hand slots.
- Grid layout: `DisplayBackpackItem` maps top hands as `2*chIdx + hand`, inventory/body/chest slots as `itemNum + 8`, chest contents as `rnChestContents[itemNum-30]`, and uses `d.IconDisplay[squareNumber].pixelX/Y` for slot geometry.
- Viewport seam: closing inventory returns to `VM_ADVENTURE`, restores movement buttons/key routing, and calls `FloorAndCeilingOnly`; in-inventory redraw paths mark `CHARFLAG_viewportChanged` and route mouth/eye/chest overlays separately.

## Source anchors

### CSBWin mirror

- `CSBWin/Character.cpp:31-76` — `DrawItem` uses `d.IconDisplay[squareNumber]` geometry, icon atlas group `GetBasicGraphicAddress(D5W + 42)`, 16x16 source offsets, and viewport/screen destination split.
- `CSBWin/Code11f52.cpp:82-115` — `GetIconGraphic` extracts 16 rows from the same `d.Word612`/graphics-42 icon atlas.
- `CSBWin/Character.cpp:1099-1218` — `DisplayBackpackItem` maps hands, inventory slots, chest slots, slot rectangles, destination width, special empty-slot icons, and final `DrawItem` call.
- `CSBWin/CSBCode.cpp:7410-7511` — `ShowHideInventory` locks open/close flow, inventory viewport backdrop, 30-slot loop, redraw flags, button routing, and return-to-adventure viewport clearing.
- `CSBWin/CSBCode.cpp:6919-6990` — `HandleClothingClick` swaps cursor/slot/chest objects, applies carry-location gates, and redraws the affected character.

### CSB/src lineage

- `CSB/src/Character.cpp:31-77` — same item icon render target/atlas contract as CSBWin.
- `CSB/src/Code11f52.cpp:109-142` — same 16x16 icon graphic extraction contract as CSBWin.
- `CSB/src/Character.cpp:1086-1208` — same backpack/grid/slot/chest mapping as CSBWin.
- `CSB/src/CSBCode.cpp:7405-7507` — same inventory open/close render and viewport return contract as CSBWin.
- `CSB/src/Character.cpp:1219-1490` — inventory-open redraw seam: portrait/title/load/backpack redraws mark viewport changed; mouth/eye/chest overlays remain separated.
- `CSB/src/Viewport.cpp:7240-7258` — mouth/food-water overlay repacks chest, blits food/water assets, and draws food/water bars into the inventory viewport.

## Parity contract locked by this pass

1. Inventory slots `0..29` are drawn into the viewport as `squareNumber = itemNum + 8`; top hands outside inventory remain `2*chIdx + hand` on the logical screen.
2. Item icons are 16x16 atlas slices selected via `d.Word612` and `GetBasicGraphicAddress(42 + group)`; slot geometry comes from `d.IconDisplay`.
3. Chest slots are inventory slots `30+` and source from `d.rnChestContents[itemNum-30]`; they must not be collapsed into body equipment slots.
4. Inventory open uses viewport backdrop graphic `17`, the 30-slot redraw loop, and inventory button list `Buttons17760`.
5. Inventory close must restore adventure mode/buttons/key routing and clear the viewport back to floor/ceiling rather than reusing stale inventory pixels.
6. Mouth/eye/chest overlays remain inside the inventory viewport redraw seam and must not be absorbed by adventuring viewport click/render code.

## Gate

```sh
python3 tools/verify_csb_v1_inventory_render_grid_source_lock.py \
  --csbwin-source /home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin \
  --csb-source /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src
```
