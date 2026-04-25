# DM1 all-graphics parity — phase 397–496: V1 HUD status-box cleanup

## Scope

Prioritised HUD parity. This batch removes invented V1 champion-HUD chrome from the party strip and binds alive/empty status-box handling closer to CHAMPION.C.

## Source anchors

- `dm7z-extract/Toolchains/Common/Source/DEFS.H:2171` says `C007_GRAPHIC_STATUS_BOX` is never used.
- `firestaff_pc34_core_amalgam.c:11459-11486` (`F0292_CHAMPION_DrawState`) clears alive champion status boxes with `C12_COLOR_DARKEST_GRAY`, then overlays shield borders/name/stat/action surfaces; dead champions use `C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION`.
- `firestaff_pc34_core_amalgam.c:11509-11518` prints name/title and then draws bar graphs; it does not draw a yellow rectangle around the active champion box.
- Empty unrecruited party slots are not iterated as champion states; only present champions have status boxes.

## Implemented

- V1 alive champion HUD no longer blits graphic 7 as a normal status-box frame.
- V1 alive champion HUD clears the 67×29 champion box to `M11_COLOR_DARK_GRAY` (`C12`) before overlays.
- V1 dead champion HUD still uses GRAPHICS.DAT graphic 8.
- Removed the invented double-yellow active champion rectangle in V1; source state remains represented by name coloring/action state, not a border.
- V1 unrecruited party slots are left undrawn instead of rendering fake empty cells/status boxes.
- Debug/V2 empty-slot visuals remain available outside V1 parity mode.

## New invariants

- `INV_GV_15C`: V1 champion HUD does not draw an invented active-slot yellow rectangle.
- `INV_GV_15D`: V1 champion HUD leaves unrecruited party slots undrawn.

## Verification

- `ctest --test-dir build --output-on-failure`
- `./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- Probe result: `455/455 invariants passed`

## Probe excerpt

```text
PASS INV_GV_15 bottom HUD renders a dedicated party/status strip instead of a single inspector blob
PASS INV_GV_15B party strip reflects source-colored champion bars when champion data exists
PASS INV_GV_15C V1 champion HUD does not draw an invented active-slot yellow rectangle
PASS INV_GV_15D V1 champion HUD leaves unrecruited party slots undrawn
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
# summary: 455/455 invariants passed
```
