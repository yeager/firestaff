# Firestaff V2 vertical-slice engine integration

This is the first real engine-side visual integration for the Wave 1 V2 slice.

## What is wired now

Enabled with `FIRESTAFF_V2_VERTICAL_SLICE=1`, the M11 game view draws these V2 assets in a real render path:

- `fs.v2.slice.viewport-frame.base`
- `fs.v2.slice.action-area.base`
- `fs.v2.slice.spell-area.base`
- `fs.v2.slice.spell-area.rune-bed`
- `fs.v2.slice.spell-area.highlight-overlay`
- `fs.v2.slice.spell-area.active-overlay`
- `fs.v2.slice.status-box.left-frame`
- `fs.v2.slice.status-box.right-frame`
- `fs.v2.slice.party-hud-cell.standard-base`
- `fs.v2.slice.party-hud-cell.highlight-overlay`

The current integration is intentionally partial:

- viewport frame is a V2 shell over the live M11 viewport
- action area is a V2 shell under the existing right-column/action text
- spell area is a V2 shell and layered bed at the original DM strip, with an extra active accent when the spell panel is open
- status-box family is used as the party HUD frame skin
- party HUD cells are composited inside each champion slot, with the highlight overlay on the active champion

## How to view it

### Probeable path

```bash
FIRESTAFF_V2_VERTICAL_SLICE=1 ./build/firestaff_m11_game_view_probe
```

To emit screenshots:

```bash
mkdir -p verification-screens/v2-vertical-slice
FIRESTAFF_V2_VERTICAL_SLICE=1 \
PROBE_SCREENSHOT_DIR=verification-screens/v2-vertical-slice \
./build/firestaff_m11_game_view_probe
```

### Runnable path

```bash
FIRESTAFF_V2_VERTICAL_SLICE=1 ./build/firestaff
```

## Asset-selection wiring

`tools/generate_v2_vertical_slice_header.py` reads the Wave 1 vertical-slice manifest and regenerates `m11_v2_vertical_slice_assets.h` from the approved 1080p exports.

## Still not integrated

Not yet wired in this pass:

- viewport inner-mask compositing
- action recess/highlight/active overlays
- portraits and full text/icon payloads
- runtime state matrices beyond the active champion HUD-cell highlight
