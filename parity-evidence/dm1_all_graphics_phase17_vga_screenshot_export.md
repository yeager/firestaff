# DM1 all-graphics phase 17 — fix probe screenshot palette export

Date: 2026-04-25 12:38 Europe/Stockholm
Scope: Firestaff V1 / screenshot evidence correctness.

## Problem

Daniel correctly flagged the in-game screenshot as looking like the wrong palette.

Root cause was not necessarily the SDL/runtime palette. The M11 probe screenshot export was wrong after the framebuffer started storing encoded bytes:

```c
raw = colorIndex | (paletteLevel << 4)
```

The probe still wrote PGM as:

```c
gray = ssFb[px] * 17;
```

That only works for bare 0..15 color indices. With encoded palette levels, the multiplication wraps in 8-bit PGM output, and the later PNG conversion interpreted those wrapped bytes as false color indices. Result: misleading, wrong-palette screenshots.

## Fix

- Added direct VGA PPM export for `party_hud_statusbox_gfx` in `probes/m11/firestaff_m11_game_view_probe.c`.
- The PPM export decodes framebuffer bytes exactly like `render_sdl_m11.c`:
  - `idx = M11_FB_DECODE_INDEX(raw)`
  - `level = M11_FB_DECODE_LEVEL(raw)`
  - `rgb = G9010_auc_VgaPaletteAll_Compat[level][idx]`
- Updated `run_firestaff_m11_game_view_probe.sh` to compile/link `vga_palette_pc34_compat.c` for the direct probe build path.
- Added `tools/convert_m11_probe_pgm_to_png.py` for recovering older legacy probe PGMs (`gray = raw * 17`) by using modular inverse `raw = gray * 241 mod 256`, then applying the real DM PC VGA palette.

## Artifacts

New direct-VGA screenshot set:

- `verification-m11/dm1-all-graphics/phase17-vga-screenshot-export-20260425-1238/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase17-vga-screenshot-export-20260425-1238/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase17-vga-screenshot-export-20260425-1238/normal/party_hud_top_190_crop_vga.png`

Recovered old phase16 corrected PNG:

- `verification-m11/dm1-all-graphics/phase16-center-door-opening-states-20260425-1224/normal/party_hud_statusbox_gfx_vga_corrected.png`
- `verification-m11/dm1-all-graphics/phase16-center-door-opening-states-20260425-1224/normal/party_hud_top_190_crop_vga_corrected.png`

Visual inspection: direct VGA export resolves the misleading palette issue in the screenshots. Remaining visual problems should be treated as geometry/compositing/viewport parity issues, not evidence-PNG palette errors.

## Verification

```sh
cmake --build build -j4
PROBE_SCREENSHOT_DIR=verification-m11/dm1-all-graphics/phase17-vga-screenshot-export-20260425-1238/normal \
  ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `362/362 invariants passed`
- CTest: `4/4 PASS`
