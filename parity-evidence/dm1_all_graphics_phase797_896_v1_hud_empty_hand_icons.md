# DM1 all-graphics parity — phase 797–896: V1 HUD empty hand icon parity

## Scope

Continue HUD/V1 parity by hardening status-box hand icon selection, especially the exact empty-hand icon indices when a hand is wounded or acting.

## Source anchors

- `dm7z-extract/Toolchains/Common/Source/DEFS.H:1952-1958`:
  - `C201_ICON_ACTION_ICON_EMPTY_HAND`
  - `C212_ICON_READY_HAND`
- `firestaff_pc34_core_amalgam.c:11335-11367` (`F0291_CHAMPION_DrawSlot`): for empty equipment/body slots, source computes `IconIndex = C212_ICON_READY_HAND + (SlotIndex << 1)` and increments it when the matching wound bit is set.
- `firestaff_pc34_core_amalgam.c:11374`: occupied slots use `F0033_OBJECT_GetIconIndex(Thing)` instead of empty-hand icons.
- `firestaff_pc34_core_amalgam.c:11391-11392`: the acting action hand overrides only the slot-box native bitmap (`C035_GRAPHIC_SLOT_BOX_ACTING_HAND`); it does not change the already-selected icon.

## Implemented

- Added `M11_GameView_GetV1StatusHandIconIndex(...)` as the single source-shaped resolver for V1 status-box hand icons.
- Empty status hands now select exact source icons:
  - ready normal/wounded: `212` / `213`
  - action normal/wounded: `214` / `215`
- Acting action hand preserves the selected empty icon while `M11_GameView_GetV1StatusHandSlotGraphic(...)` continues to select graphic `35` for the box.
- Occupied wounded hands use the object icon resolver (`F0033_OBJECT_GetIconIndex` mirror), not the empty-hand icon family.
- `m11_draw_v1_status_hand_slot(...)` now consumes the shared icon-index helper so renderer and probes cannot drift.

## New invariants

- `INV_GV_15K`: empty normal ready/action hands use icons `212/214`.
- `INV_GV_15L`: ready-hand wound advances only the ready icon to `213`.
- `INV_GV_15M`: action-hand wound advances only the action icon to `215`.
- `INV_GV_15N`: acting action hand changes the slot box but keeps the source wounded empty icon.
- `INV_GV_15O`: occupied wounded hand uses object icon index `16` for synthetic weapon 0 instead of an empty-hand icon.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe -j2`
- `ctest --test-dir build --output-on-failure`
- `./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- Probe result: `466/466 invariants passed`

## Probe excerpt

```text
PASS INV_GV_15F V1 champion HUD ready/action hands use normal slot-box graphic when idle
PASS INV_GV_15G V1 champion HUD action hand switches to graphic 35 for the acting champion
PASS INV_GV_15H V1 champion HUD ready-hand wound selects graphic 34 only for ready hand
PASS INV_GV_15I V1 champion HUD action-hand wound selects graphic 34 when idle
PASS INV_GV_15J V1 champion HUD acting action hand overrides wound graphic with graphic 35
PASS INV_GV_15K V1 champion HUD empty normal hands use source icons 212/214
PASS INV_GV_15L V1 champion HUD ready-hand wound advances empty icon to 213 only
PASS INV_GV_15M V1 champion HUD action-hand wound advances empty icon to 215 only
PASS INV_GV_15N V1 champion HUD acting hand changes the box graphic but keeps the source wounded empty icon
PASS INV_GV_15O V1 champion HUD occupied wounded hand uses F0033 object icon instead of empty-hand icon
# summary: 466/466 invariants passed
```
