# DM1 all-graphics parity — phase 1677–1696: V1 object-icon source zones

## Scope

Make DM1 object-icon source atlas resolution explicit and probe-visible for both action-hand cells and compact status-hand rendering.

## Source anchors

- DM1 object icons are 16×16 cells.
- Firestaff uses GRAPHICS.DAT graphic base `F0042` for object-icon pages.
- There are 32 icons per graphic page (`16` columns × `2` rows).
- Empty-hand icon index `201` resolves to graphic `48`, local cell `9`, source rect `(144,0,16,16)`.
- Icon index `16` resolves to graphic `42`, local cell `16`, source rect `(0,16,16,16)`.

## Implemented

- Added `M11_GameView_GetV1ObjectIconSourceZone(...)`.
- Routed `m11_draw_dm_object_icon_index(...)` through the helper instead of recomputing the atlas math inline.
- Added invariant coverage for a cross-page empty-hand icon and a row-1 base-page icon.
- Drawing behavior and action palette remap remain unchanged.

## New invariant

- `INV_GV_300K`: V1 object icon source zones resolve 16x16 cells across F0042+ graphics.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300K V1 object icon source zones resolve 16x16 cells across F0042+ graphics
```
