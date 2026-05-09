# Pass453 DM1 V1 Hall panel_visible delta classification

Status: `FIRESTAFF_FRAME_STATE_MISMATCH_PANEL_NOT_VISIBLE`.

This is a diagnostic classification only, not a pixel-parity claim.

Evidence artifact: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-comparator-diff-20260509/panel_visible_delta_manifest.json`

Summary:

- Firestaff `panel_visible/fullframe` is hash-identical to Firestaff `candidate_select/fullframe`, so the Firestaff comparator input did not advance to a visibly panel-open state.
- Original `panel_visible` has the C101 panel crop present at `xywh=[80,85,144,73]`.
- The paired comparator dimensions match (`320x200` fullframe, `144x73` panel crop), so the observed failure is not a dimension/scale mismatch.
- Small-offset template checks around the expected panel crop remain high-delta, so this is not explained by a small crop-origin error.
- External DM1 PC34 original data is hash-locked in the diagnostic manifest and rechecked by the pass453 verifier.

Gate: `python3 tools/verify_pass453_dm1_v1_hall_panel_visible_delta_classification.py`
