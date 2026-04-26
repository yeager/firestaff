# DM1 all-graphics parity — phase 2097–2116: V1 status/slot/shield graphic ids

## Scope

Expose and route the source GRAPHICS.DAT ids used by the V1 party-status frames, hand-slot boxes, and party shield borders.

## Source anchors

- `C007_GRAPHIC_STATUS_BOX` = graphic 7.
- `C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION` = graphic 8.
- `C033_GRAPHIC_SLOT_BOX_NORMAL` = graphic 33.
- `C034_GRAPHIC_SLOT_BOX_WOUNDED` = graphic 34.
- `C035_GRAPHIC_SLOT_BOX_ACTING_HAND` = graphic 35.
- `C037/C038/C039` are the party shield / fire shield / spell shield border graphics.

## Implemented

- Added probe-visible helpers for the normal/dead status-box graphic ids.
- Added probe-visible helpers for normal/wounded/acting hand-slot box graphic ids.
- Added probe-visible helpers for party/fire/spell shield border graphic ids.
- Routed existing V1 status-slot and shield selection/load paths through those helpers instead of direct enum use.

## Updated invariant

- `INV_GV_300W`: V1 status slot and shield frame graphics use source `C007/C008/C033-C035/C037-C039` ids.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 508/508 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300W V1 status slot and shield frame graphics use source C007/C008/C033-C035/C037-C039 ids
```
