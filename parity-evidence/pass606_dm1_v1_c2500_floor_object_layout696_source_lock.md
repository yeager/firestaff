# Pass 606 - DM1 V1 C2500 floor-object layout-696 source lock

## Scope

This pass closes a DM1 V1 viewport/world-visuals source-lock gap in the C2500 floor-object/item anchor tables. Firestaff had drifted to the ReDMCSB static fallback `G3024_s_LayoutData09` values for C2500, but DM1 PC 3.4 / I34E loads `C696_GRAPHIC_LAYOUT` from `GRAPHICS.DAT` at runtime.

The canonical data source used here is:

- `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT`
- sha256: `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`
- reconstructed table: `data/zones_h_reconstruction.json`, records `C2500..C2567`

No `DUNGEON.DAT` or `TITLE` content was compared for this slice.

## ReDMCSB source anchors

Primary source audit under `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`:

- `COORD.C:2074-2109` clears `MASK0x8000_SHIFT_OBJECTS_AND_CREATURES` and applies supplied object/creature shifts in `F0637_GetProportionalZone`.
- `COORD.C:2498-2570` loads layout ranges in `F0639_LoadLayoutRanges`, loads graphic index `C696_GRAPHIC_LAYOUT` in `F0640_LoadLayoutData`, and initializes it from `F0641_InitializeLayout`.
- `DUNVIEW.C:373` defines `G2028_ac_ViewSquareIndexTo`, the row mapping used by viewport squares.
- `DUNVIEW.C:4547-4582` documents `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` per-square content order.
- `DUNVIEW.C:4922-4923` gates visible floor objects/items before drawing.
- `DUNVIEW.C:5071-5078` selects `(C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES) + (row * 4) + viewCell` for normal object placement.
- `DEFS.H:3517` defines `MASK0x8000_SHIFT_OBJECTS_AND_CREATURES`; `DEFS.H:4228-4236` defines the C2500/C2900/C3200 viewport content zone families.

## Change

`src/engine/m11_game_view.c` now uses the canonical DM1 PC 3.4 layout-696 C2500 coordinates for both the five-row renderer helper and the full 17-row raw verifier helper. `tools/verify_v1_viewport_source_zone_tables.py` now hash-locks the canonical `GRAPHICS.DAT`, checks the ReDMCSB layout-load and F0115 routing anchors, and fails if Firestaff's C2500/C2900/C3200 tables drift from the source dump.

## Verification

- `python3 tools/verify_v1_viewport_source_zone_tables.py`
- `ctest --test-dir build-pass606-c2500-source-lock -R "v1_viewport_source_zone_tables|m11_viewport_state_probe" --output-on-failure`
- `cmake --build build-pass606-c2500-source-lock --target firestaff_m11_viewport_state_probe`
- `git diff --check`
- committed-diff secret scan with high-signal token/key regex

## Non-goals

This pass does not change mouth visual blits, creature projectile live-runtime logic, V2, CSB, DM2, or Nexus code. It does not claim a new pixel-capture parity gate; it source-locks the data path and verifier that feed the existing viewport C2500 object placement helpers.
