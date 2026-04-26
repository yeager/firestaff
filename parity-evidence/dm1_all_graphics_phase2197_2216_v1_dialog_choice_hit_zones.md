# DM1 all-graphics parity — phase 2197–2216: V1 dialog choice hit zones

## Scope

Expose and route the V1 dialog pointer hit zones from the same source choice layout used for visible choice text.

## Source anchors

- Choice hit rows are derived from the source choice text zones by using the same x/width and a 17 px vertical activation band starting 6 px above the text baseline.
- This preserves the existing source dialog behavior while removing the duplicate hard-coded switch in the pointer path.

## Implemented

- Added `M11_GameView_GetV1DialogChoiceHitZone()`.
- Routed dialog mouse hit testing through the helper.

## Updated invariant

- `INV_GV_300AB`: V1 dialog pointer hit zones derive from source choice text zones.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 513/513 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300AB V1 dialog pointer hit zones derive from source choice text zones
```
