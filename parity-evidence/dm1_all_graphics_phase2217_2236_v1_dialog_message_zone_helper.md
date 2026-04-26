# DM1 all-graphics parity — phase 2217–2236: V1 dialog message zone helper

## Scope

Expose the reconstructed V1 dialog message rectangles directly instead of keeping the C469/C471 geometry split between the vertical-origin helper and the message-width helper.

## Source anchors

- `DIALOG.C:F0427_DIALOG_Draw` chooses `C469_ZONE_DIALOG` for single-choice/dialog-dismiss overlays.
- The 2/3/4-choice paths choose `C471_ZONE_DIALOG` before printing centered message text.
- Reconstructed layout records:
  - `C469`: child top/left `(112,49)`, parent bottom/right `(188,73)` → `(112,49,77,25)`.
  - `C471`: child top/left `(112,32)`, parent bottom/right `(188,36)` → `(112,32,77,5)`.

## Implemented

- Added `M11_GameView_GetV1DialogMessageZone()`.
- Routed `M11_GameView_GetV1DialogMessageWidth()` through the zone helper.
- Routed the C469/C471 message vertical-origin helpers through the same zone helper.

## Updated invariant

- `INV_GV_300Z`: V1 dialog message zones and vertical origins use source `C469/C471` geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 513/513 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300Z V1 dialog message zones and vertical origins use source C469/C471 geometry
```
