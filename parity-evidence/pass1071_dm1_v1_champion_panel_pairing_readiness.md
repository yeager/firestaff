# Pass1071 DM1 V1 champion-panel pairing readiness

Status: `BLOCKED_ORIGINAL_FOUR_CHAMPION_HUD_AND_SINGLE_STATUS_PANEL_CAPTURE_MISSING`

This verifier fingerprints the already-tracked pass1053 original
champion-candidate panel artifacts and the existing Firestaff V1 HUD PPMs.
It is a readiness/blocker record only and explicitly makes no parity claim.

## Inputs

- pass1053 manifest: `verification-screens/pass1053-dm1-original-champion-candidate-panel/manifest.json`
- pass1053 report: `parity-evidence/pass1053_dm1_v1_original_champion_candidate_panel_capture.md`
- Firestaff HUD PPM directory: `verification-m11/lane3-inventory-followup-20260428-0914`

## Fingerprint Result

- pass1053 manifest status OK: `True`
- pass1053 frame files OK: `True`
- pass1053 crop files OK: `True`
- Firestaff V1 HUD PPMs OK: `True`
- Overall fingerprint OK: `True`

## Firestaff PPMs

| File | Dimensions | SHA256 | Status |
|---|---:|---|---|
| `verification-m11/lane3-inventory-followup-20260428-0914/party_hud_four_champions_vga.ppm` | `[320, 200]` | `d995c4991e5b973ea98c7eedd2d13bdca0e624061983da99131ea5110ddd17a9` | OK |
| `verification-m11/lane3-inventory-followup-20260428-0914/party_hud_statusbox_gfx_vga.ppm` | `[320, 200]` | `860bc022785b2567eedfa552a99e103d02c573b432446c679a273d96d0a18363` | OK |

## Original Frames

| Label | Dimensions | SHA256 | Status |
|---|---:|---|---|
| `start_before_portrait_click` | `[320, 200]` | `50bead319e59bd42c9b5af6e4a39275e6cfc7a02fee96e6f6b766e575858fabc` | OK |
| `candidate_select_after_click_111_82` | `[320, 200]` | `e4b373078be6aa0c27e793ccd476b6e886b34ef0c4b063c6d2274815351af53e` | OK |
| `resurrect_terminal_hud_after_click_130_115` | `[320, 200]` | `7523b67fa765ffb02a088bf8dbb0c2ba3630fcf5bcc2fb11f956b4e442b52b8f` | OK |

## Blocker

- Missing original four-champion HUD capture for same-state pairing.
- Missing original single-status-panel capture for same-state pairing.
- Therefore this pass is readiness evidence only, not parity evidence.

## Non-Claims

- No Firestaff-vs-original pixel diff is performed.
- No same-state champion-panel parity row is promoted.
- No game code is changed or exercised by this verifier.

Manifest: `parity-evidence/verification/pass1071_dm1_v1_champion_panel_pairing_readiness/manifest.json`
