# Candidate: viewport runtime 224x136 crop evidence

## Scope

This candidate locks Firestaff's V1 runtime viewport aperture and capture crop path to the DM1 PC 3.4 source rectangle:

- `x = 0`
- `y = 33`
- `w = 224`
- `h = 136`

Source anchors already present in `m11_game_view.c`:

- `COORD.C G2067_i_ViewportScreenX = 0`
- `COORD.C G2068_i_ViewportScreenY = 33`
- `DEFS.H C112_BYTE_WIDTH_VIEWPORT * 2 = 224`
- `DEFS.H C136_HEIGHT_VIEWPORT = 136`

## What changed

- Kept and tightened the existing WIP in `verification-screens/capture_firestaff_ingame_series.c`:
  - full-frame captures are still written as before;
  - each deterministic in-game capture now also writes `<name>_viewport_224x136.ppm`;
  - capture fails hard if `M11_GameView_GetViewportRect()` does not return `(0,33,224,136)`.
- Extended `probes/m11/firestaff_m11_game_view_probe.c` with invariants:
  - `INV_GV_12D`: runtime viewport crop fits inside 320x200 and has exact area `224*136 = 30464`;
  - `INV_GV_12E`: two same-state draws hash-identically inside the 224x136 viewport crop.
- Updated `run_capture_screenshots.sh` so optional PNG conversion includes the new `*_viewport_224x136.ppm` files when `magick`/`convert` is available.

## Runtime/capture evidence

Command:

```sh
./run_capture_screenshots.sh "$HOME/.firestaff/data"
```

Result:

```text
wrote 6 in-game screenshots and 6 viewport crops to verification-screens
Screenshots captured in verification-screens
```

Generated crop headers checked locally:

```text
01_ingame_start_latest_viewport_224x136.ppm          P6 224 136 255
02_ingame_turn_right_latest_viewport_224x136.ppm     P6 224 136 255
03_ingame_move_forward_latest_viewport_224x136.ppm   P6 224 136 255
04_ingame_spell_panel_latest_viewport_224x136.ppm    P6 224 136 255
05_ingame_after_cast_latest_viewport_224x136.ppm     P6 224 136 255
06_ingame_inventory_panel_latest_viewport_224x136.ppm P6 224 136 255
```

## Gates

```sh
cmake -S . -B build
cmake --build build --target firestaff_m11_game_view_probe firestaff capture_ingame_series -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
cmake --build build --target firestaff_m11_phase_a_probe firestaff_m11_audio_probe -- -j2
ctest --test-dir build --output-on-failure
./run_capture_screenshots.sh "$HOME/.firestaff/data"
```

Observed:

- build targets: passed
- M11 game-view probe: `537/537 invariants passed`
- ctest: `5/5` passed after building the probe executables required by this fresh build dir
- capture: wrote six full-frame PPMs and six deterministic 224x136 viewport PPM crops

## What this proves

- The V1 runtime viewport API is source-bound to the DM1 PC 3.4 `(0,33,224,136)` aperture.
- The capture tool now emits deterministic 224x136 viewport crops from the same rendered frame as the full screenshot.
- Same-state rendering is byte-stable within that crop, so future emulator-overlay comparison can use these crops as reproducible Firestaff-side inputs.

## What this does **not** claim

- No 1:1 original emulator overlay parity is claimed here; this candidate does not include a matching DM1 PC 3.4 emulator viewport dump for the same state.
- No cadence/timing parity is claimed.
- No broad dungeon renderer rewrite is claimed. This is viewport aperture/capture/probe groundwork plus runtime guardrails.

## Merge risk

Low. The runtime source rectangle was already wired in `m11_game_view.c`; this candidate mainly adds probe coverage and capture artifacts. The only script behavior change is optional conversion of newly generated crop PPMs when an image converter exists.
