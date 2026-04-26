# DM1 all-graphics parity — phase 2797–2816: V1 explosion/viewport text zones

## Scope

Expose layout-696 zone ids and resolved geometry for the D0C explosion-pattern anchor and viewport-centered text anchor.

## Source anchors

ReDMCSB `DEFS.H` / layout-696:

- `C004_ZONE_EXPLOSION_PATTERN_D0C` — child of viewport-sized parent `C003`; a `32×29` pattern resolves at `(0,0,32,29)`.
- `C006_ZONE_VIEWPORT_CENTERED_TEXT` — centered under `C004`; a representative `77×15` text region resolves at `(73,60,77,15)` in the `224×136` viewport parent.

Values cross-checked against `zones_h_reconstruction.json` with `tools/resolve_dm1_zone.py`.

## Implemented

- Added `M11_GameView_GetV1ExplosionPatternD0CZoneId()` / `M11_GameView_GetV1ExplosionPatternD0CZone()`.
- Added `M11_GameView_GetV1ViewportCenteredTextZoneId()` / `M11_GameView_GetV1ViewportCenteredTextZone(contentW, contentH, ...)`.

## Updated invariants

- `INV_GV_300AO`: asserts `C004`/`C006` ids and representative resolved geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — passed.
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — `526/526 invariants passed`.
- `ctest --test-dir build --output-on-failure` — `5/5` tests passed.
