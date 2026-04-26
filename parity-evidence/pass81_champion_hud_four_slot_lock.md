# Pass81 — V1 champion HUD four-slot lock

Goal: prioritize and lock the DM1 V1 bottom HUD with recruited champions visible in all four source status-box slots.

## Source anchors

- ReDMCSB status-box input/zones are anchored by `C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS`, `C187_ZONE_CHAMPION_0_STATUS_BOX_BAR_GRAPHS`, and `C159_ZONE_CHAMPION_0_STATUS_BOX_NAME`.
- Existing layout reconstruction keeps the 696 table in `zones_h_reconstruction.json` and `tools/extract_zones_layout_696.py`.
- M11 helpers under test:
  - `M11_GameView_GetV1StatusBoxZone(...)`
  - `M11_GameView_GetV1StatusNameZone(...)`
  - `M11_GameView_GetV1StatusBarZone(...)`

## What changed

- Added probe invariant `INV_GV_15E10` in `probes/m11/firestaff_m11_game_view_probe.c`.
- The invariant constructs a four-champion party and verifies:
  - slot 2 status box at `(150,160)` with `67x29` source footprint,
  - slot 3 status box at `(219,160)` with `67x29` source footprint,
  - source-colored names are rendered in compact name zones,
  - slot 2 HP bar uses champion-color red,
  - slot 3 mana bar uses champion-color light blue.
- Added visual screenshot dump `party_hud_four_champions_vga.ppm` for quick inspection.

## Evidence

- Screenshot: `verification-screens/pass81-champion-hud-priority/party_hud_four_champions_vga.png`
- Raw PPM: `verification-screens/pass81-champion-hud-priority/party_hud_four_champions_vga.ppm`
- Gate: `PROBE_SCREENSHOT_DIR=verification-screens/pass81-champion-hud-priority ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- Result: `# summary: 563/563 invariants passed`

## Status

This locks the champion HUD priority visually and in probe form. It does not claim full DM1 V1 parity; original DOS route parity remains gated by Pass80.
