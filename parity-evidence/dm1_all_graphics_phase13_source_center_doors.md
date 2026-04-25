# DM1 all-graphics phase 13 — source-bound center doors

Date: 2026-04-25 11:55 Europe/Stockholm
Scope: Firestaff V1 / DM1 viewport center door zones.

## Change

Added a narrow source-bound door pass for center/front doors:

- D1C, D2C, D3C closed/closing center doors now draw original door frame and door panel assets into layout-696 resolved zones.
- Door pass only applies to `DUNGEON_ELEMENT_DOOR` cells that are not open.
- Wall pass remains restricted to wall/fakewall cells, so closed doors no longer become stone walls.

This is not a full `F0111_DUNGEONVIEW_DrawDoor` port yet. It handles the closed/visible center-door case using source placements and transparency.

## Source anchors

ReDMCSB:

- `DUNVIEW.C F0111_DUNGEONVIEW_DrawDoor`
- `DUNVIEW.C F0122/F0124/F0127` center door calls
- `M633_GRAPHIC_FIRST_DOOR_SET = 246`
- `G0693_ai_DoorNativeBitmapIndex_Front_D3LCR`
- `G0694_ai_DoorNativeBitmapIndex_Front_D2LCR`
- `G0695_ai_DoorNativeBitmapIndex_Front_D1LCR`
- `C722/C723/C724/C725/C726/C727/C730/C733` door frame zones
- `M625_ZONE_DOOR_D3C`, `M628_ZONE_DOOR_D2C`, `M631_ZONE_DOOR_D1C`

## Resolved placements used

From `tools/resolve_dm1_zone.py` / `COORD.C F0635_` semantics:

### D1C

- top frame graphic `91` -> zone `C733`: `x=61 y=12 w=102 h=4`
- left frame graphic `87` -> zone `C726`: `x=44 y=13 w=25 h=94`
- right frame graphic `87` -> zone `C727`: `x=155 y=13 w=25 h=94`
- door panel graphic `248` -> zone `M631_ZONE_DOOR_D1C`: `x=64 y=16 w=96 h=86`

### D2C

- top frame graphic `92` -> zone `C730`: `x=77 y=21 w=70 h=3`
- left frame graphic `88` -> zone `C724`: `x=65 y=21 w=18 h=65`
- right frame graphic `88` -> zone `C725`: `x=141 y=21 w=18 h=65`
- door panel graphic `247` -> zone `M628_ZONE_DOOR_D2C`: `x=80 y=24 w=64 h=59`

### D3C

- left frame graphic `89` -> zone `C722`: `x=82 y=27 w=10 h=42`
- right frame graphic `89` -> zone `C723`: `x=132 y=27 w=10 h=42`
- door panel graphic `246` -> zone `M625_ZONE_DOOR_D3C`: `x=90 y=30 w=44 h=38`

## Artifacts

Generated screenshot set:

- `verification-m11/dm1-all-graphics/phase13-source-center-doors-20260425-1155/normal/*.pgm`
- `verification-m11/dm1-all-graphics/phase13-source-center-doors-20260425-1155/normal/*.png`

Quick crop:

- `verification-m11/dm1-all-graphics/phase13-source-center-doors-20260425-1155/normal/party_hud_top_190_crop.png`

Visual inspection: net improvement, no obvious regression. Center door/frame pieces are now anchored to original zones instead of independently scaled placeholder rectangles. Remaining issues: full DUNVIEW ordering, side-door cases, opening-state zone shifts, ornaments/buttons, palette/masking verification, and UI clipping.

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `362/362 invariants passed`
- CTest: `4/4 PASS`

## Next step

Port more of `F0111_DUNGEONVIEW_DrawDoor` semantics:

1. Door type/door set selection (`DoorSet0/1`, door type).
2. Opening-state zone shifts and vertical vs horizontal doors.
3. Door ornaments/buttons/destroyed mask.
4. Side door positions (`D1L/R`, `D2L/R`, `D3L/R`, `D3L2/R2`).
