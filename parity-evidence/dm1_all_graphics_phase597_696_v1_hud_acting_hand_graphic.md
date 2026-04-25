# DM1 all-graphics parity — phase 597–696: V1 HUD acting-hand slot graphic

## Scope

Tighten the previous HUD hand-slot pass by making the status-box hand-slot graphic selection explicit and probeable, especially the acting champion action-hand variant.

## Source anchors

- `firestaff_pc34_core_amalgam.c:11392` sets `C035_GRAPHIC_SLOT_BOX_ACTING_HAND` when `P0614_ui_SlotIndex == C01_SLOT_ACTION_HAND` and the champion ordinal equals `G0506_ui_ActingChampionOrdinal`.
- `firestaff_pc34_core_amalgam.c:11550-11553` draws ready/action hand slots for non-inventory champions through `F0291_CHAMPION_DrawSlot`.
- `firestaff_pc34_core_amalgam.c:11618-11624` redraws the action hand when the action-hand state changes.

## Implemented

- Added `M11_GameView_GetV1StatusHandSlotGraphic(...)` so the renderer and probes share the same source-shaped decision:
  - idle ready hand → graphic `33` (`C033_GRAPHIC_SLOT_BOX_NORMAL`)
  - idle action hand → graphic `33`
  - acting champion action hand → graphic `35` (`C035_GRAPHIC_SLOT_BOX_ACTING_HAND`)
  - dead/missing/non-present champion → `0`
- Refactored V1 status hand rendering to use that helper instead of duplicating the graphic-selection branch inline.
- Kept wound-specific graphic `34` out of this pass until the status-hand wound bits are wired explicitly; avoiding a guessed wound mapping is intentional.

## New invariants

- `INV_GV_15F`: V1 champion HUD ready/action hands use normal slot-box graphic when idle.
- `INV_GV_15G`: V1 champion HUD action hand switches to graphic 35 for the acting champion.

## Verification

- `ctest --test-dir build --output-on-failure`
- `./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- Probe result: `458/458 invariants passed`

## Probe excerpt

```text
PASS INV_GV_15 bottom HUD renders a dedicated party/status strip instead of a single inspector blob
PASS INV_GV_15B party strip reflects source-colored champion bars when champion data exists
PASS INV_GV_15C V1 champion HUD does not draw an invented active-slot yellow rectangle
PASS INV_GV_15D V1 champion HUD leaves unrecruited party slots undrawn
PASS INV_GV_15E V1 champion HUD draws source ready/action hand slot zones inside the status box
PASS INV_GV_15F V1 champion HUD ready/action hands use normal slot-box graphic when idle
PASS INV_GV_15G V1 champion HUD action hand switches to graphic 35 for the acting champion
PASS INV_GV_150 side-cell creature drawing code path exercised safely
PASS INV_GV_151 BlitScaledMirror produces horizontally flipped output
PASS INV_GV_152 BlitScaledMirror skips transparent pixels
PASS INV_GV_153 V1 depth dimming encodes palette level in upper bits
PASS INV_GV_154 V1 depth dimming preserves original colour index
PASS INV_GV_155 M11_FB_ENCODE/DECODE round-trip for all index/level combos
PASS INV_GV_156 front-facing D1 GiantScorpion view selects native front bitmap (M618+0)
PASS INV_GV_157 side-facing D1 GiantScorpion falls back to front (no SIDE bit, no FLIP_NON_ATTACK)
PASS INV_GV_158 back-facing D2 GiantScorpion falls back to derived front D2 (no BACK bit)
PASS INV_GV_159 front-facing attack GiantScorpion falls back to front (no ATTACK bit, no FLIP_ATTACK)
PASS INV_GV_156 Font init produces unloaded state
PASS INV_GV_157 Original DM1 font loads from GRAPHICS.DAT
PASS INV_GV_158 Font DrawChar 'A' produces visible pixels
PASS INV_GV_159 Font DrawString 'HELLO' renders visible text
PASS INV_GV_344 projectile travel: runtime-only projectile is reflected in viewport cell summary
PASS INV_GV_347 projectile detonation: explosion is fireball type and appears in viewport cell summary
# summary: 458/458 invariants passed
```
