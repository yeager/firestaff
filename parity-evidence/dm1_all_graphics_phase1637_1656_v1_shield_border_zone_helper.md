# DM1 all-graphics parity — phase 1637–1656: V1 shield-border zone helper

## Scope

Make party shield/fire shield/spell shield border overlay placement probe-visible as an explicit V1 status-box child zone.

## Source anchors

- GRAPHICS.DAT `C037_GRAPHIC_BORDER_PARTY_SHIELD`: `67×29`.
- GRAPHICS.DAT `C038_GRAPHIC_BORDER_PARTY_FIRESHIELD`: `67×29`.
- GRAPHICS.DAT `C039_GRAPHIC_BORDER_PARTY_SPELLSHIELD`: `67×29`.
- These overlays occupy the same footprint as the `C007/C008` champion status-box frame.
- Existing `M11_GameView_GetV1StatusBoxZone(...)` resolves slot 0 as `(12,160,67,29)` and slot 3 as `(219,160,67,29)`.

## Implemented

- Added `M11_GameView_GetV1StatusShieldBorderZone(...)` as an explicit wrapper over the V1 status-box zone.
- Routed V1 shield-border blitting through the helper.
- Added invariant coverage for slot 0 and slot 3 border geometry.
- Shield graphic priority from the previous helper remains unchanged: spell > fire > party.

## New invariant

- `INV_GV_15T`: V1 status shield border zone reuses C007 status box footprint.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_15T V1 status shield border zone reuses C007 status box footprint
```
