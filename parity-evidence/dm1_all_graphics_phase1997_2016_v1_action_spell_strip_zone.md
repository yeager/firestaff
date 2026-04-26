# DM1 all-graphics parity — phase 1997–2016: V1 action+spell strip zone

## Scope

Make the combined V1 right-column action/spell strip explicit and route the partial-asset fallback clear through that source-derived union.

## Source anchors

- ReDMCSB/ZONES.H keeps the action area at `C011_ZONE_ACTION_AREA` (224,45,87,45).
- The spell area sits directly below at `C013_ZONE_SPELL_AREA` (224,90,87,25).
- Their combined right-column strip is therefore (224,45,87,70), spanning y=45..114.

## Implemented

- Added `M11_GameView_GetV1ActionSpellStripZone()` as the union of the existing action/spell source-zone helpers.
- Routed the V1 partial original-frame fallback clear through that helper instead of recomputing height inline.
- Reused `M11_GameView_GetV1ActionAreaClearColor()` for the fallback clear color.

## Updated invariant

- `INV_GV_300R`: V1 action+spell strip union covers source C011/C013 right-column zones.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300R V1 action+spell strip union covers source C011/C013 right-column zones
```
