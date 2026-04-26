# DM1 all-graphics parity — phase 2717–2736: V1 champion icon zones

## Scope

Expose the layout-696 source zone ids and resolved clipped rectangles for the four top-right champion icon hit/display zones.

## Source anchors

ReDMCSB `DEFS.H` / layout-696:

- `C113_ZONE_CHAMPION_ICON_TOP_LEFT`
- `C114_ZONE_CHAMPION_ICON_TOP_RIGHT`
- `C115_ZONE_CHAMPION_ICON_BOTTOM_RIGHT`
- `C116_ZONE_CHAMPION_ICON_BOTTOM_LEFT`

`zones_h_reconstruction.json` resolves these through parent `C112` (`19×14`) under `C111`, so the native `16×16` champion icon graphic is clipped to `16×14` at:

- slot 0: `(281,0,16,14)`
- slot 1: `(301,0,16,14)`
- slot 2: `(301,15,16,14)`
- slot 3: `(281,15,16,14)`

Cross-check command:

```text
python3 - <<'PY'
from tools.resolve_dm1_zone import resolve
for z in (113,114,115,116): print(z, resolve(z,16,16))
PY
```

## Implemented

- Added `M11_GameView_GetV1ChampionIconZoneId(slot)` returning `C113..C116`.
- Added `M11_GameView_GetV1ChampionIconZone(slot, ...)` exposing the resolved clipped rectangles.

## Updated invariants

- `INV_GV_300AK`: asserts first/last zone ids, invalid-slot rejection, and representative slot geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — passed.
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — `522/522 invariants passed`.
- `ctest --test-dir build --output-on-failure` — `5/5` tests passed.
