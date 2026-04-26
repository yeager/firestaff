# Phase 70 — viewport source base and draw-order probe seam

## Scope

Small runtime-invariant pass only.  This does **not** claim 1:1 original overlay parity.

## Change

Added probe-visible viewport source seams:

- `M11_GameView_GetV1ViewportBaseGraphic(...)` locks the V1 viewport base to:
  - layer 0: source ceiling graphic `C079` / GRAPHICS.DAT `79`, `224x39`, destination `(0,0)` inside the viewport aperture.
  - layer 1: source floor graphic `C078` / GRAPHICS.DAT `78`, `224x97`, destination `(0,39)` inside the viewport aperture.
- `M11_GameView_GetV1ViewportSourceDrawOrder*()` locks the current source-backed renderer pass order from base/pits/floor ornaments/side+front walls/wall ornaments/stairs/fields through side+center doors, door ornaments, destroyed masks, and buttons.
- `firestaff_m11_game_view_probe` adds `INV_GV_414` and `INV_GV_415` to verify those seams.  When GRAPHICS.DAT is available, `INV_GV_414` also verifies the loaded `79` and `78` asset dimensions.

## Crop-manifest evidence

`./run_capture_screenshots.sh "$HOME/.firestaff/data"` was rerun after the probe pass.  It produced the deterministic 6 full frames and 6 viewport crops at the source aperture `224x136`.

Viewport crop rows from this run:

```text
01_ingame_start_latest_viewport_224x136.ppm        b418163c0b8310ab35273b62ec5f0e042e586098a0227a7c64c51cfa168bcee2
02_ingame_turn_right_latest_viewport_224x136.ppm   6db0edeeba2118636cc70b037504378081d6e1299a268fceab7fab1f7cce529f
03_ingame_move_forward_latest_viewport_224x136.ppm 30514a9fd8a8d578e3a44e3faf072d4025697c2e5d360a7a51a963fea794678a
04_ingame_spell_panel_latest_viewport_224x136.ppm  30514a9fd8a8d578e3a44e3faf072d4025697c2e5d360a7a51a963fea794678a
05_ingame_after_cast_latest_viewport_224x136.ppm   30514a9fd8a8d578e3a44e3faf072d4025697c2e5d360a7a51a963fea794678a
06_ingame_inventory_panel_latest_viewport_224x136.ppm a2192a96553db14fd3da2166a75249b22428aa2c0a72a4dd3402280137f0587e
```

`03`/`04`/`05` matching is acceptable for this fixture: the viewport aperture is unchanged by right-column spell UI transitions after the deterministic movement step.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff capture_ingame_series -- -j2` — PASS
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — PASS, `558/558 invariants`
- `ctest --test-dir build --output-on-failure` — initially failed because selective build had not produced `firestaff_m11_phase_a_probe` and `firestaff_m11_audio_probe`; after building those two probe targets, PASS `5/5`.
- `./run_capture_screenshots.sh "$HOME/.firestaff/data"` — PASS, 6 full frames + 6 viewport crops, 4 unique viewport hashes.
- secret scan over diff — PASS, no hits.

## Remaining blocker

Original DM1 emulator crops for the exact deterministic route are still missing, so this pass locks source graphic IDs, asset availability, and renderer draw-order inputs only.  It does not close viewport pixel/content parity.
