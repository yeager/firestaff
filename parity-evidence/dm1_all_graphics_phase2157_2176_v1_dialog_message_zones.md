# DM1 all-graphics parity — phase 2157–2176: V1 dialog message zones

## Scope

Expose and route the source V1 dialog message width and vertical text origins used for single-choice and multi-choice dialog layouts.

## Source anchors

- `C469` single-choice message zone resolves to a 77 px source message band.
- `C471` multi-choice message zone resolves to the same 77 px source message band.
- The source text baseline/line-step math places one-line/two-line single-choice dialog text at framebuffer y `96/92`, and multi-choice text at y `70/66` for the DM1 viewport origin `(0, 33)`.

## Implemented

- Added `M11_GameView_GetV1DialogMessageWidth()`.
- Added `M11_GameView_GetV1DialogSingleChoiceMessageTextY()` and `M11_GameView_GetV1DialogMultiChoiceMessageTextY()`.
- Routed V1 source-dialog text drawing through the new public helpers.

## Updated invariant

- `INV_GV_300Z`: V1 dialog message width and vertical origins use source `C469/C471` zones.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 511/511 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300Z V1 dialog message width and vertical origins use source C469/C471 zones
```
