# pass376 DM1 V1 original-vs-Firestaff measurement overlays

Status: MEASUREMENT_ONLY_NOT_PARITY_CLAIM

This package records deterministic 224x136 viewport overlay measurements for the
existing pass376 original DM1 PC 3.4 viewport crops against current Firestaff
viewport crops. The artifacts are useful as visual-debug evidence, but they do
not claim same-state parity. The measured deltas remain large and still require
source-backed interpretation before any row can be promoted.

Artifact root:

- `parity-evidence/overlays/pass376_firestaff_pairing/`

Plan:

- `plan.json` schema: `pass70_viewport_pair_compare.v1`
- `viewport_xywh`: `[0, 33, 224, 136]`
- `crop_xywh_for_224x136_inputs`: `[0, 0, 224, 136]`
- pair count: 6
- blockers: none
- parity_claimed: false

Measurements:

| Scene | Differing pixels | Total pixels | Delta |
|---|---:|---:|---:|
| `01_ingame_start` | 22149 | 30464 | 72.7055% |
| `02_ingame_turn_right` | 25009 | 30464 | 82.0936% |
| `03_ingame_move_forward` | 23497 | 30464 | 77.1304% |
| `04_ingame_spell_panel` | 23497 | 30464 | 77.1304% |
| `05_ingame_after_cast` | 27108 | 30464 | 88.9837% |
| `06_ingame_inventory_panel` | 28480 | 30464 | 93.4874% |

Generated files:

- `01_ingame_start_viewport_original_vs_firestaff.stats.json`
- `01_ingame_start_viewport_original_vs_firestaff.mask.png`
- `02_ingame_turn_right_viewport_original_vs_firestaff.stats.json`
- `02_ingame_turn_right_viewport_original_vs_firestaff.mask.png`
- `03_ingame_move_forward_viewport_original_vs_firestaff.stats.json`
- `03_ingame_move_forward_viewport_original_vs_firestaff.mask.png`
- `04_ingame_spell_panel_viewport_original_vs_firestaff.stats.json`
- `04_ingame_spell_panel_viewport_original_vs_firestaff.mask.png`
- `05_ingame_after_cast_viewport_original_vs_firestaff.stats.json`
- `05_ingame_after_cast_viewport_original_vs_firestaff.mask.png`
- `06_ingame_inventory_panel_viewport_original_vs_firestaff.stats.json`
- `06_ingame_inventory_panel_viewport_original_vs_firestaff.mask.png`

Verification notes:

- Every `*.mask.png` is a 224x136 RGB PNG.
- Every stats file uses the same keys:
  `delta_percent`, `differing_pixels`, `firestaff`, `firestaff_sha256`,
  `honesty`, `mask_path`, `original`, `original_sha256`, `pass`,
  `region_xywh`, `tolerance_per_channel`, `tool`, and `total_pixels`.
- The stats `honesty` field explicitly records that each diff is measurement
  only and is not a parity claim without source-backed interpretation.
