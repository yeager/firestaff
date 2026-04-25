# DM1 all-graphics parity — phase 497–596: V1 HUD status-box hand slots

## Scope

Continue HUD-priority work by wiring the ready/action hand slots inside each V1 champion status box.

## Source anchors

- `dm7z-extract/Toolchains/Common/Source/DEFS.H:3800-3807` defines status-box hand zones `C211..C218`.
- `zones_h_reconstruction.json` / layout-696 places those zones at:
  - ready hand: champion status parent + `(4,10)`
  - action hand: champion status parent + `(24,10)`
  - parent slot zone size: `16×16`
- `firestaff_pc34_core_amalgam.c:11335-11396` (`F0291_CHAMPION_DrawSlot`) draws normal/wounded/acting slot-box graphics and source object/empty-hand icons.
- `firestaff_pc34_core_amalgam.c:11550-11553` draws ready/action hand slots for non-inventory champions via `F0291`.
- `firestaff_pc34_core_amalgam.c:11618-11624` redraws action hand + right-column action icon when the action hand state changes.

## Implemented

- Added V1 status-box hand zone constants from layout-696:
  - `M11_V1_STATUS_READY_HAND_X = 4`
  - `M11_V1_STATUS_ACTION_HAND_X = 24`
  - `M11_V1_STATUS_HAND_Y = 10`
- Added source-shaped status hand rendering for present living champions:
  - ready hand slot box
  - action hand slot box
  - empty ready/action icons (`C212`, `C214`)
  - object icon lookup when a hand contains an item
  - acting hand selects `C035_GRAPHIC_SLOT_BOX_ACTING_HAND`
- Kept these hand slots separate from the right-column action icon cells; they are different source surfaces.

## New invariant

- `INV_GV_15E`: V1 champion HUD draws source ready/action hand slot zones inside the status box.

## Verification

- `ctest --test-dir build --output-on-failure`
- `./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- Probe result: `456/456 invariants passed`

## Probe excerpt

```text
PASS INV_GV_15 bottom HUD renders a dedicated party/status strip instead of a single inspector blob
PASS INV_GV_15B party strip reflects source-colored champion bars when champion data exists
PASS INV_GV_15C V1 champion HUD does not draw an invented active-slot yellow rectangle
PASS INV_GV_15D V1 champion HUD leaves unrecruited party slots undrawn
PASS INV_GV_15E V1 champion HUD draws source ready/action hand slot zones inside the status box
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
# summary: 456/456 invariants passed
```
