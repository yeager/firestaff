# Pass 82 — Firestaff V1 source-zone overlay probe

Date: 2026-04-26

## Goal

Strengthen the original-screen/overlay evidence path without claiming fake parity. This pass takes the current deterministic Firestaff V1 320×200 screenshot series, draws source-relevant DM1 zones on each frame, crops those zones, and measures static crops against extracted GRAPHICS.DAT/ReDMCSB anchor images where an anchor exists.

## Source evidence

- `reference-artifacts/provenance.json` records the source-anchor path from `GRAPHICS.DAT` extraction and ReDMCSB/DM1 references (`DEFS.H`, `COORD.C`).
- Static anchors used here:
  - `reference-artifacts/anchors/0000_viewport_full_frame.png` — C000 viewport aperture, 224×136.
  - `reference-artifacts/anchors/0010_action_area.png` — C010 action area, 87×45.
  - `reference-artifacts/anchors/0009_spell_area_backdrop.png` — C009 spell-area backdrop, 87×25.
  - `reference-artifacts/anchors/0020_panel_empty.png` — C020 inventory panel empty, 144×73.
- Zone coordinates are the current DM1 V1 screen coordinates used by the parity matrix:
  - viewport `0,33,224,136`
  - action area `224,45,87,45`
  - spell area `224,90,87,25`
  - right column action/spell `224,45,87,70`
  - inventory panel `80,53,144,73`
  - message area `0,169,224,31`

## Tool and outputs

- Tool: `tools/pass82_firestaff_source_zone_overlay_probe.py`
- Stats: `parity-evidence/overlays/pass82/pass82_firestaff_source_zone_overlay_stats.json`
- Overlays: `parity-evidence/overlays/pass82/*_source_zones_overlay.png`
- Crops and masks: `parity-evidence/overlays/pass82/*_{zone}.png` and `*_anchor_mask.png`

Command:

```sh
python3 tools/pass82_firestaff_source_zone_overlay_probe.py
```

Result:

```text
scenes=6, problems=[]
```

## Static-anchor measurements

These numbers are measurements only. Dynamic viewport contents and route/state differences are expected to differ from a static anchor; do not read them as parity failure or parity success without source interpretation.

| scene | viewport C000 | action C010 | spell C009 | inventory C020 |
| --- | ---: | ---: | ---: | ---: |
| 01 ingame_start | 93.9305% | 25.8493% | 35.9080% | 72.9642% |
| 02 ingame_turn_right | 95.1123% | 25.8493% | 35.9080% | 72.9642% |
| 03 ingame_move_forward | 93.9863% | 25.8493% | 35.9080% | 98.2306% |
| 04 ingame_spell_panel | 93.9863% | 41.2261% | 35.9080% | 98.2306% |
| 05 ingame_after_cast | 93.9863% | 25.8493% | 35.9080% | 98.2306% |
| 06 ingame_inventory_panel | 93.9437% | 25.8493% | 35.9080% | 15.6868% |

## Interpretation

- The pass adds bounded, repo-relative visual evidence for every current deterministic Firestaff V1 screenshot.
- The overlays make HUD, viewport, action/spell, inventory, and message regions inspectable without relying on local absolute paths.
- The static-anchor deltas highlight likely dynamic overlays/palette/state differences, not a source-backed code mismatch by themselves.
- No gameplay/M10 behavior was changed.
