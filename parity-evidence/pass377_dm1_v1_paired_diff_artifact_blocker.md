# Pass377 — DM1 V1 paired diff artifact blocker narrowing

Status: `BLOCKED_PASS377_PAIRED_DIFF_REVIEW_METADATA_READY_SEMANTIC_ORIGINAL_BLOCKED`

## Decision

Blocker #4 is narrowed to review-only pairing metadata: all six Firestaff viewport inputs and all six current original viewport crops are present and dimension-checked, and the pass70 pairing plan is materialized. Paired diffs/masks/stats are deliberately not run because the original route is not semantic-clean yet.

## Source audit

- `COORD.C:1693-1724` — paired diff inputs must be 224x136 viewport crops from the PC34 viewport at x=0,y=33 ok=`True`
- `DUNVIEW.C:2962-3002,3048-3092` — Firestaff/original crops must compare the composed G0296 viewport buffer, not arbitrary full-frame pixels ok=`True`
- `DRAWVIEW.C:709-858` — original-side captures are promotable only after the source presentation seam ok=`True`
- `VIDEODRV.C:3566-3582` — PC34 slot 9 presents the viewport with the expected 224-wide source and 320-wide screen stride ok=`True`

## Dependency blockers

- `blocker_1_original_true_stop_transcript` `BLOCKED_PASS360_ORIGINAL_RUNTIME_TRUE_STOP_BLOCKER_NARROWED` — strict original FIRES F0128 -> F0097/VIDRV true-stop is still required before original frames are promotable ok=`True`
- `blocker_2_labelled_original_full_frames` `BLOCKED_PASS376_ORIGINAL_FRAMES_CROPS_NARROWED` — labels/raw original 320x200 frames and 224x136 crops exist, but duplicate hashes/pass86 mismatches still block semantic promotion ok=`True`
- `blocker_3_original_viewport_crops` `BLOCKED_PASS376_ORIGINAL_OVERLAY_RUNTIME_ARTIFACTS_MISSING` — 224x136 original crops are present as review-only inputs, but cannot be promoted until pass86/semantic route succeeds ok=`True`
- `firestaff_route_source_locked` `PASS372_DM1_V1_MOVEMENT_RUNTIME_ROUTE_SOURCE_LOCKED` — Firestaff movement/capture side is not the active route blocker ok=`True`

## Firestaff-side artifacts

- `ingame_start` `verification-screens/01_ingame_start_latest_viewport_224x136.png` dims=`[224, 136]` manifest_ppm=`True` ok=`True`
- `ingame_turn_right` `verification-screens/02_ingame_turn_right_latest_viewport_224x136.png` dims=`[224, 136]` manifest_ppm=`True` ok=`True`
- `ingame_move_forward` `verification-screens/03_ingame_move_forward_latest_viewport_224x136.png` dims=`[224, 136]` manifest_ppm=`True` ok=`True`
- `ingame_spell_panel` `verification-screens/04_ingame_spell_panel_latest_viewport_224x136.png` dims=`[224, 136]` manifest_ppm=`True` ok=`True`
- `ingame_after_cast` `verification-screens/05_ingame_after_cast_latest_viewport_224x136.png` dims=`[224, 136]` manifest_ppm=`True` ok=`True`
- `ingame_inventory_panel` `verification-screens/06_ingame_inventory_panel_latest_viewport_224x136.png` dims=`[224, 136]` manifest_ppm=`True` ok=`True`

## Original review-only artifacts

- `ingame_start` `verification-screens/pass376-original-dm1-viewports/viewport_224x136/01_ingame_start_original_viewport_224x136.png` dims=`[224, 136]` manifest_ppm=`True` review_only_ok=`True`
- `ingame_turn_right` `verification-screens/pass376-original-dm1-viewports/viewport_224x136/02_ingame_turn_right_original_viewport_224x136.png` dims=`[224, 136]` manifest_ppm=`True` review_only_ok=`True`
- `ingame_move_forward` `verification-screens/pass376-original-dm1-viewports/viewport_224x136/03_ingame_move_forward_original_viewport_224x136.png` dims=`[224, 136]` manifest_ppm=`True` review_only_ok=`True`
- `ingame_spell_panel` `verification-screens/pass376-original-dm1-viewports/viewport_224x136/04_ingame_spell_panel_original_viewport_224x136.png` dims=`[224, 136]` manifest_ppm=`True` review_only_ok=`True`
- `ingame_after_cast` `verification-screens/pass376-original-dm1-viewports/viewport_224x136/05_ingame_after_cast_original_viewport_224x136.png` dims=`[224, 136]` manifest_ppm=`True` review_only_ok=`True`
- `ingame_inventory_panel` `verification-screens/pass376-original-dm1-viewports/viewport_224x136/06_ingame_inventory_panel_original_viewport_224x136.png` dims=`[224, 136]` manifest_ppm=`True` review_only_ok=`True`

## Pairing/diff plan

- plan: `parity-evidence/overlays/pass377/pass377_pairing_plan.json`
- original-side blockers from pass70: `[]`
- review-only metadata materialized: `True`
- semantic-clean original route required: `True`
- masks/stats materialized: `False`
- diffs run: `False` (requires promotable semantic-clean original pair; no parity claim)

## Non-claims

- No original-vs-Firestaff pixel parity is claimed.
- No paired diff masks/stats are claimed because the current original crops are review-only, not semantic-clean.
- No original true-stop transcript or semantically matched original frame is claimed.

Manifest: `parity-evidence/verification/pass377_dm1_v1_paired_diff_artifact_blocker/manifest.json`
