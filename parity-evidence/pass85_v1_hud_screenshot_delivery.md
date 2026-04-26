# Pass 85 — V1 champion HUD screenshot delivery

This pass packages small, reviewable top-row champion HUD screenshots from the current Firestaff V1 capture state.

Honesty: these are current Firestaff evidence artifacts only. They do not claim original-runtime pixel parity.

- stats: `parity-evidence/overlays/pass85_v1_hud_screenshot_delivery/pass85_v1_hud_screenshot_delivery_stats.json`
- checksum manifest: `parity-evidence/overlays/pass85_v1_hud_screenshot_delivery/pass85_v1_hud_screenshot_delivery_sha256.tsv`
- HUD crop box: `[12, 0, 274, 29]` (x, y, width, height)
- scenes: 3

## Delivered artifacts

- `party_hud_with_champions` from `verification-screens/07_party_hud_with_champions.png`
  - raw HUD crop: `parity-evidence/overlays/pass85_v1_hud_screenshot_delivery/party_hud_with_champions_top_row_hud_raw.png`
  - zoned HUD crop: `parity-evidence/overlays/pass85_v1_hud_screenshot_delivery/party_hud_with_champions_top_row_hud_zoned.png`
- `spell_panel_with_champions` from `verification-screens/08_spell_panel_with_champions.png`
  - raw HUD crop: `parity-evidence/overlays/pass85_v1_hud_screenshot_delivery/spell_panel_with_champions_top_row_hud_raw.png`
  - zoned HUD crop: `parity-evidence/overlays/pass85_v1_hud_screenshot_delivery/spell_panel_with_champions_top_row_hud_zoned.png`
- `four_champion_priority` from `verification-screens/pass81-champion-hud-priority/party_hud_four_champions_vga.png`
  - raw HUD crop: `parity-evidence/overlays/pass85_v1_hud_screenshot_delivery/four_champion_priority_top_row_hud_raw.png`
  - zoned HUD crop: `parity-evidence/overlays/pass85_v1_hud_screenshot_delivery/four_champion_priority_top_row_hud_zoned.png`

## Notes

- The raw crop is the top-row champion HUD rectangle only, kept small for quick visual review.
- The zoned crop uses the same DM1 PC 3.4 layout-696 zone geometry as pass 83, clipped to the HUD row.
- Checksums in the TSV manifest fingerprint the exact bytes committed in this pass.
