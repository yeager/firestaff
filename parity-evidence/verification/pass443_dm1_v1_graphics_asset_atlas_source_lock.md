# Pass443 — DM1 V1 GRAPHICS.DAT asset-atlas source lock

## Scope

Daniel's release-test note said the latest release looks better but many graphical assets are wrong. This pass treats that as a graphics-asset correctness investigation, not just a wall-set index cleanup.

Primary evidence remains ReDMCSB WIP20210206 PC34/I34E. Greatstone is secondary cross-check/provenance only.

## Greatstone provenance/context

Greatstone overview page `g_dm.html` identifies the relevant references as:

- Dungeon Master PC 3.4: `dungeon.dat` maps, `graphics.dat` dungeon graphics, `song.dat`, `title`, `end`.
- Dungeon Master PC 3.4 `(en-fr-ge)`: `dungeon.dat` English maps, `dungeonf.dat` French maps, `dungeong.dat` German maps, shared `graphics.dat` dungeon graphics, `song.dat`, `title`, `end`.

Important: PC34_MULTI dungeon maps are not treated as identical to plain PC34. This pass does not lock dungeon-map invariants from PC34_MULTI; it only compares the shared `graphics.dat` dungeon-graphics atlas labels.

## Atlas cross-check

Greatstone PC34 `graphics.dat` and PC34_MULTI `graphics.dat` agree exactly for the dungeon graphics indices locked here:

- 49..79: floor pits, invisible pits, ceiling pits, wall masks, teleporter, fluxcage, floor, ceiling.
- 86..125: door frame strips, wall panels, and stairs.

The multi-language atlas differs in interface-language labels elsewhere, but not in these dungeon graphics ranges.

## Concrete mismatch found and fixed

Firestaff had correct ReDMCSB wall-set block constants for 86..125, but `m11_draw_stairs_asset()` still routed stairs through legacy wall-panel slots:

- `M11_GFX_STAIRS_DOWN_D2 = 93` → Greatstone/ReDMCSB: wall right side 0
- `M11_GFX_STAIRS_UP_D2 = 94` → wall left side 0
- `M11_GFX_STAIRS_DOWN_D1 = 95` → wall right side 1
- `M11_GFX_STAIRS_UP_D1 = 96` → wall left side 1

ReDMCSB/Greatstone PC34 place actual front stairs in the materialized wall-set block:

- stairs up front D2/D1: 111 / 113
- stairs down front D2/D1: 118 / 120

Fix: route the stair helper to `M11_GFX_DM1_STAIRS_*_FRONT_*` constants and pass the selected index through `m11_wallset_graphic_index_for_state()` so nonzero map wall sets materialize correctly.

## Gate added

`tools/verify_pass443_dm1_v1_graphics_asset_atlas_source_lock.py`

The gate verifies:

- ReDMCSB PC34/I34E constants for 49..79 and 86..125.
- ReDMCSB `F0095_DUNGEONVIEW_LoadWallSet` materialization formula.
- Embedded Greatstone PC34/PC34_MULTI agreement for 49..79 and 86..125.
- Firestaff no longer maps stairs to wall-panel slots 93..96.
- Firestaff stair drawing remaps through the current map wall-set helper.

## Remaining graphics-asset risk

This pass fixed one real wrong-asset family: stairs. Other families still worth targeted probes:

- item/HUD icon sheet regions 42..48, especially empty-hand and inventory/body placeholders;
- viewport object sprites 498..583 versus object-icon sheets 42..48;
- creature palette replacement colors and derived bitmap scaling;
- teleporter/fluxcage overlays and wall-mask composition order;
- any runtime fallback branch that draws procedural rectangles while `assetsAvailable` is true.
