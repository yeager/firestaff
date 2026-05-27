# DM2 V1 Phase 8 — Viewport/Pixel Gate
**Pass:** H2312
**Date:** 2026-05-26
**Schema:** `firestaff.dm2_v1.viewport_pixel_gate.v1`

## Overview
Canonical pixel-capture gate for DM2 V1 (Skullkeep). Defines capture regions and geometry in 320×200 game-pixel space. Each fixture has a locked state label and source evidence from SKULL.ASM. expected_sha256 is empty (TBD) for all fixtures; downstream probe must populate from known-good runs.
## Fixture Table

| Fixture ID | State | Region | Geometry | Status |
|-----------|-------|--------|----------|--------|
| `dm2_v1_first_dungeon_viewport_full` | dm2_first_dungeon_entrance | viewport_full | 320×200 @ (0,0) | TBD (no hash) |
| `dm2_v1_first_dungeon_dungeon_view` | dm2_first_dungeon_entrance | dungeon_view | 320×144 @ (0,28) | TBD (no hash) |
| `dm2_v1_first_dungeon_viewport_center` | dm2_first_dungeon_entrance | viewport_center | 224×144 @ (48,28) | TBD (no hash) |
| `dm2_v1_first_dungeon_status_bar` | dm2_first_dungeon_entrance | status_bar | 320×28 @ (0,0) | TBD (no hash) |
| `dm2_v1_first_dungeon_action_strip` | dm2_first_dungeon_entrance | chrome_bottom | 320×28 @ (0,172) | TBD (no hash) |
| `dm2_v1_first_dungeon_panel_right` | dm2_first_dungeon_entrance | panel_right | 80×144 @ (240,28) | TBD (no hash) |
| `dm2_v1_forward_viewport_full` | dm2_dungeon_forward | viewport_full | 320×200 @ (0,0) | TBD (no hash) |
| `dm2_v1_lshape_viewport_full` | dm2_dungeon_l_shape | viewport_full | 320×200 @ (0,0) | TBD (no hash) |
| `dm2_v1_inventory_viewport_full` | dm2_inventory_open | viewport_full | 320×200 @ (0,0) | TBD (no hash) |

## Capture Conventions
- **Pixel format:** PPM P6 (24-bit RGB)
- **Game pixel space:** 320×200 (V1 original)
- **Palette:** VGA 256-color indexed (palette in GRAPHICS.DAT / GDAT)
- **Capture tool:** firestaff_headless_dm2_v1_viewport_capture_probe
- **Capture env:** SDL_VIDEODRIVER=dummy, SDL_AUDIODRIVER=dummy
- **Game data:** ~/.firestaff/data/dm2

## Platform Notes
- **V1_original:** 320×200 pixel-perfect, palette-indexed VGA
- **V2_filtered:** 320×200 + CRT scanlines + palette correction (same pixel geometry)
- **V2_upscaled:** 3200×2000 10× upscale, same geometry
- **V2_modern:** 1920×1080 HD, remapped geometry (not 1:1 with V1)
