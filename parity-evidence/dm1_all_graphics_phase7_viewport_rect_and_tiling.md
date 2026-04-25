# DM1 all-graphics phase 7 — source viewport rect + remove noisy placeholder tiling

Date: 2026-04-25 10:52 Europe/Stockholm
Scope: Firestaff V1 / DM1 PC 3.4 normal viewport composition.

## Changes

### Source-faithful viewport rectangle

Normal V1 now uses the original DM1 PC 3.4 viewport rectangle:

- `x = 0`
- `y = 33`
- `w = 224`
- `h = 136`

Source anchors already documented in `m11_game_view.c`:

- ReDMCSB `COORD.C`: `G2067_i_ViewportScreenX = 0`
- ReDMCSB `COORD.C`: `G2068_i_ViewportScreenY = 33`
- ReDMCSB / `DEFS.H`: viewport byte width/pixel height = `112 bytes * 2 = 224`, `136 px`
- recovered layout table: `zones_h_reconstruction.json`, from `GRAPHICS.DAT` entry `696`

The old smaller Firestaff viewport `(12,24,196,118)` is no longer the normal V1 anchor.

### Removed noisy placeholder texture tiling from normal V1

The old Firestaff viewport path tiled wall/floor GRAPHICS.DAT strips across procedural trapezoids. That produced chaotic speckled wall/floor noise and did not match original DM1 `DRAWVIEW` ownership.

That placeholder tiling is now debug-only (`state->showDebugHUD`). Normal V1 keeps the cleaner source-size viewport while waiting for the proper source-bound `DRAWVIEW` wall/floor/ceiling placement pass.

## Artifacts

Generated normal V1 screenshot set:

- `verification-m11/dm1-all-graphics/phase7b-clean-viewport-20260425-1052/normal/*.pgm`
- `verification-m11/dm1-all-graphics/phase7b-clean-viewport-20260425-1052/normal/*.png`

Quick top/viewport crop:

- `verification-m11/dm1-all-graphics/phase7b-clean-viewport-20260425-1052/normal/party_hud_top_170_crop.png`

Visual inspection: composition is less chaotic after removing the speckled placeholder tiling, and the viewport is in the correct DM1 upper-left screen position. Remaining problems are still substantial: right UI unfinished, bottom HUD cropped/awkward, and viewport contents still not source-faithful `DRAWVIEW` output.

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `362/362 invariants passed`
- CTest: `4/4 PASS`

## Remaining work

This is still not 1:1 viewport parity. Next required work:

- implement source-bound `DRAWVIEW` wall/floor/ceiling draw order instead of Firestaff procedural room shapes
- exact original-vs-Firestaff viewport crop comparison for fixed map/x/y/facing/light
- right-side panel/action/spell zone alignment and `PASS`/icon layout cleanup
- bottom champion HUD / inventory panel source-zone rebuild
