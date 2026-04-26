# DM1 all-graphics parity — phase 1457–1476: V1 status shield border priority

## Scope

Make the V1 party status-box shield border selection probe-visible and source-backed instead of keeping the priority inline at the draw site.

## Source anchors

- ReDMCSB `INVNTORY.C` / shield overlay handling uses the party shield-defense type to choose one of the status-box border overlays.
- GRAPHICS.DAT overlay graphics:
  - `C037` party shield border
  - `C038` fire shield border
  - `C039` spell shield border
- Existing parity priority is highest active shield type: spell > fire > party.

## Implemented

- Added `M11_GameView_GetV1StatusShieldBorderGraphic(...)`.
- Routed V1 status-box shield border drawing through that helper.
- Added invariant coverage for no shield, party shield, fire-over-party, and spell-over-fire-over-party priorities.

## New invariant

- `INV_GV_15P`: V1 status shield border graphic priority follows spell/fire/party source order.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `484/484 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_15P V1 status shield border graphic priority follows spell/fire/party source order
# summary: 484/484 invariants passed
```
