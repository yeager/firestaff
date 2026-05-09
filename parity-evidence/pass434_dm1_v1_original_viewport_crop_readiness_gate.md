# Pass434 — DM1 V1 original viewport crop readiness gate

Status: `PASS_PASS434_ORIGINAL_VIEWPORT_CROP_READINESS`

This gate keeps the original viewport crop/source-lock prerequisites executable without claiming pixel parity or requiring a live DOSBox run.

## ReDMCSB source audit

- `CLIKMENU.C:142-174,180-347` `F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty` — ok=`True`; accepted turns/steps mutate the party tuple before any original viewport crop can be promoted
- `DUNVIEW.C:8318-8611` `F0128_DUNGEONVIEW_Draw_CPSF` — ok=`True`; the 224x136 crop is meaningful only after F0128 draws G0296 for direction/X/Y
- `DRAWVIEW.C:709-858` `F0097_DUNGEONVIEW_DrawViewport` — ok=`True`; capture readiness is locked to the PC34 viewport-present seam, not setup echo/menu text

## Existing crop artifacts

- manifest: `verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv`; rows=`6`; rows_all_224x136=`True`
- six PNG/PPM crop pairs present and manifest-bound: `True`

## Executable tooling

- pass86 self-test return code: `0`
- pass376 prior manifest status: `BLOCKED_PASS376_ORIGINAL_FRAMES_CROPS_NARROWED`; ok=`True`
- capture script crop locks: `{'path': 'scripts/dosbox_dm1_original_viewport_reference_capture.sh', 'has_normalize_only': True, 'has_pillow_crop_0_33_224_169': True, 'has_imagemagick_crop': True, 'has_manifest_geometry_check': True}`
- DOSBox available: `True`; xvfb-run available: `True`; required for this gate: `False`

## Blocker honesty

- Existing pass376 crops are mechanical/review inputs only until the original route is semantically clean.
- This gate does not launch DOSBox and does not claim original-vs-Firestaff pixel parity.
- If DOSBox/live emulator is unavailable, promotion remains blocked at the capture/promotion step, not at this readiness gate.

Manifest: `parity-evidence/verification/pass434_dm1_v1_original_viewport_crop_readiness_gate/manifest.json`
