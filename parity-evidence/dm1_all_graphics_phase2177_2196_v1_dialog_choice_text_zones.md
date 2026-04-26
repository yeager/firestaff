# DM1 all-graphics parity — phase 2177–2196: V1 dialog choice text zones

## Scope

Expose and route the V1 source dialog choice text zones for 1-, 2-, 3-, and 4-choice dialogs.

## Source anchors

- Single-choice bottom choice text uses the wide bottom zone (`C462` family): viewport-relative `(16, 110, 192)`.
- Two-choice dialog uses wide top/bottom choice rows.
- Three-choice dialog uses wide top row plus left/right bottom half-width rows.
- Four-choice dialog uses the two-by-two half-width choice layout (`C464-C467` family).

## Implemented

- Added `M11_GameView_GetV1DialogChoiceTextZone()`.
- Routed source dialog choice text drawing through the helper instead of embedding layout constants in the draw switch.

## Updated invariant

- `INV_GV_300AA`: V1 dialog choice text zones expose source `C462-C467` layout cases.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 512/512 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300AA V1 dialog choice text zones expose source C462-C467 layout cases
```
