# DM1 all-graphics parity — phase 697–796: V1 HUD wounded hand graphics

## Scope

Continue HUD parity by wiring source wound bits into V1 champion status-box hand slot graphic selection.

## Source anchors

- `dm7z-extract/Toolchains/Common/Source/DEFS.H:736-737`:
  - `MASK0x0001_WOUND_READY_HAND`
  - `MASK0x0002_WOUND_ACTION_HAND`
- `firestaff_pc34_core_amalgam.c:11335-11396` (`F0291_CHAMPION_DrawSlot`): hand/body slots use wounded graphic `C034_GRAPHIC_SLOT_BOX_WOUNDED` when the corresponding wound bit is set.
- `firestaff_pc34_core_amalgam.c:11391-11392`: acting champion action hand overrides the selected box with `C035_GRAPHIC_SLOT_BOX_ACTING_HAND`.
- `memory_champion_state_pc34_compat.h` stores the same wound bitfield as `ChampionState_Compat.wounds`.

## Implemented

- `M11_GameView_GetV1StatusHandSlotGraphic(...)` now honors source wound bits:
  - ready-hand wound (`0x0001`) → graphic `34`
  - action-hand wound (`0x0002`) → graphic `34`
  - acting action hand → graphic `35`, overriding wound graphic exactly like source `F0291`
  - idle/unwounded hand → graphic `33`
- Renderer path already consumes this helper, so the V1 HUD status hand slots now reflect hand wounds without duplicating branch logic.

## New invariants

- `INV_GV_15H`: ready-hand wound selects graphic 34 only for ready hand.
- `INV_GV_15I`: action-hand wound selects graphic 34 when idle.
- `INV_GV_15J`: acting action hand overrides wound graphic with graphic 35.

## Verification

- `ctest --test-dir build --output-on-failure`
- `./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- Probe result: `461/461 invariants passed`

## Probe excerpt

```text
PASS INV_GV_15 bottom HUD renders a dedicated party/status strip instead of a single inspector blob
PASS INV_GV_15B party strip reflects source-colored champion bars when champion data exists
PASS INV_GV_15C V1 champion HUD does not draw an invented active-slot yellow rectangle
PASS INV_GV_15D V1 champion HUD leaves unrecruited party slots undrawn
PASS INV_GV_15E V1 champion HUD draws source ready/action hand slot zones inside the status box
PASS INV_GV_15F V1 champion HUD ready/action hands use normal slot-box graphic when idle
PASS INV_GV_15G V1 champion HUD action hand switches to graphic 35 for the acting champion
PASS INV_GV_15H V1 champion HUD ready-hand wound selects graphic 34 only for ready hand
PASS INV_GV_15I V1 champion HUD action-hand wound selects graphic 34 when idle
PASS INV_GV_15J V1 champion HUD acting action hand overrides wound graphic with graphic 35
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
# summary: 461/461 invariants passed
```
