# DM1 all-graphics phase 14 — door set/type selection

Date: 2026-04-25 12:05 Europe/Stockholm
Scope: Firestaff V1 / DM1 viewport door panel selection.

## Change

Center door panels are no longer hardcoded to door set 0 graphics `246/247/248`.

`M11_ViewportCell` now records the door thing type (`0` or `1`) from `DungeonDoor_Compat.type`. The source-bound center-door renderer picks the door panel graphic like ReDMCSB `F0095_DUNGEONVIEW_LoadDoorSet`:

- map `DoorSet0` for door type 0
- map `DoorSet1` for door type 1
- D3 panel = `M633_GRAPHIC_FIRST_DOOR_SET + doorSet * 3 + 0`
- D2 panel = `M633_GRAPHIC_FIRST_DOOR_SET + doorSet * 3 + 1`
- D1 panel = `M633_GRAPHIC_FIRST_DOOR_SET + doorSet * 3 + 2`

Frame graphics remain wall-set frame assets (`87/88/89/91/92`) in the resolved layout zones.

## Source anchors

- `DUNVIEW.C F0095_DUNGEONVIEW_LoadDoorSet`
- `M633_GRAPHIC_FIRST_DOOR_SET = 246`
- `G0693_ai_DoorNativeBitmapIndex_Front_D3LCR[2]`
- `G0694_ai_DoorNativeBitmapIndex_Front_D2LCR[2]`
- `G0695_ai_DoorNativeBitmapIndex_Front_D1LCR[2]`
- `DungeonMapDesc_Compat.doorSet0`
- `DungeonMapDesc_Compat.doorSet1`
- `DungeonDoor_Compat.type`

## Artifacts

Generated screenshot set:

- `verification-m11/dm1-all-graphics/phase14-door-set-selection-20260425-1205/normal/*.pgm`
- `verification-m11/dm1-all-graphics/phase14-door-set-selection-20260425-1205/normal/*.png`

Quick crop:

- `verification-m11/dm1-all-graphics/phase14-door-set-selection-20260425-1205/normal/party_hud_top_190_crop.png`

Visual inspection: no obvious regression versus the previous center-door pass. Door remains centered and aligned; panel art now follows map/door type selection.

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

Port the opening-state semantics from `F0111_DUNGEONVIEW_DrawDoor`:

- closed state uses base zone
- opening states offset/shift via zone index increments and `MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR`
- horizontal vs vertical door handling differs
- then add side door zones and door ornaments/buttons.
