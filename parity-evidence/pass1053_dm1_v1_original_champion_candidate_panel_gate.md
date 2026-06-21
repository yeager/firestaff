# Pass1053 DM1 V1 original champion candidate-panel gate

Status: `PASS1053_ORIGINAL_CHAMPION_CANDIDATE_PANEL_GATE`

This CTest gate keeps the existing original-PC34 candidate/resurrect-panel
evidence reproducible. It verifies manifest status, image hashes, crop
dimensions, source anchors, and the existing Firestaff-side champion HUD
reference captures.

## Inputs

- Original evidence: `verification-screens/pass1053-dm1-original-champion-candidate-panel`
- Source report: `parity-evidence/pass1053_dm1_v1_original_champion_candidate_panel_capture.md`
- Firestaff-side references:
  - `verification-m11/lane3-inventory-followup-20260428-0914/party_hud_four_champions_vga.ppm` dims=[320, 200] ok=True
  - `verification-m11/lane3-inventory-followup-20260428-0914/party_hud_statusbox_gfx_vga.ppm` dims=[320, 200] ok=True

## Original frames

| Label | SHA256 | Crops OK | Status |
|---|---|---:|---|
| `start_before_portrait_click` | `50bead319e59bd42` | True | OK |
| `candidate_select_after_click_111_82` | `e4b373078be6aa0c` | True | OK |
| `resurrect_terminal_hud_after_click_130_115` | `7523b67fa765ffb0` | True | OK |

## Non-claims

- This is not a Firestaff-vs-original pixel diff.
- This does not close the full four-champion HUD/status-panel capture gap.
- This does not promote any same-state champion-panel parity row.

Manifest: `parity-evidence/verification/pass1053_dm1_v1_original_champion_candidate_panel_gate/manifest.json`
