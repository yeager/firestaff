# Pass 69 — deterministic viewport crop manifest

## Scope

This pass does **not** claim DM1 original pixel parity. It adds deterministic visual evidence for the current Firestaff runtime viewport renderer so future emulator/original overlays can diff stable bytes instead of ad-hoc screenshots.

## Change

`run_capture_screenshots.sh` now writes `verification-screens/capture_manifest_sha256.tsv` after producing the existing 6 full-frame PPMs and 6 source-aperture viewport crops.

Manifest columns:

```text
kind<TAB>filename<TAB>width<TAB>height<TAB>bytes<TAB>sha256
```

The viewport rows are the DM1 PC 3.4 viewport aperture `(0,33,224,136)` from `M11_GameView_GetViewportRect`, matching DEFS/COORD-backed C007 geometry.

## Local run evidence

Command:

```sh
./run_capture_screenshots.sh "$HOME/.firestaff/data"
```

Result:

```text
wrote 6 in-game screenshots and 6 viewport crops to .../verification-screens
Capture manifest: .../verification-screens/capture_manifest_sha256.tsv (6 viewport crops, 5 unique hashes)
Screenshots captured in .../verification-screens
```

Viewport crop hashes from this run:

```text
01_ingame_start_latest_viewport_224x136.ppm       b418163c0b8310ab35273b62ec5f0e042e586098a0227a7c64c51cfa168bcee2
02_ingame_turn_right_latest_viewport_224x136.ppm  6db0edeeba2118636cc70b037504378081d6e1299a268fceab7fab1f7cce529f
03_ingame_move_forward_latest_viewport_224x136.ppm 30514a9fd8a8d578e3a44e3faf072d4025697c2e5d360a7a51a963fea794678a
04_ingame_spell_panel_latest_viewport_224x136.ppm 778467aa00c1f6108059c8bac78aab8ebb60010a82f80a5579ad72c1adf9c7cc
05_ingame_after_cast_latest_viewport_224x136.ppm  778467aa00c1f6108059c8bac78aab8ebb60010a82f80a5579ad72c1adf9c7cc
06_ingame_inventory_panel_latest_viewport_224x136.ppm a2192a96553db14fd3da2166a75249b22428aa2c0a72a4dd3402280137f0587e
```

`04` and `05` intentionally match in this fixture because the viewport crop is unchanged by the spell-cast UI transition; the full-frame rows remain available in the manifest for right-column/chrome diffs.

## Blocker retained

No original DM1 emulator crop is present in-tree for these exact views, so this pass only locks Firestaff candidate bytes. Pixel parity remains blocked on an original 320×200 capture/crop for the same deterministic route.
