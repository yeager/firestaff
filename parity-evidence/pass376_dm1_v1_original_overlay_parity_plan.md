# Pass376 — DM1 V1 original overlay parity artifact plan

Status: `BLOCKED_PASS376_ORIGINAL_OVERLAY_RUNTIME_ARTIFACTS_MISSING`

## Decision

Do not claim representative movement/HUD/viewport overlay parity yet. Pass372 closes the Firestaff movement route, but pass360 still leaves the original FIRES true-stop/capture side unproven.

## ReDMCSB source audit

- `CLIKMENU.C:142-174,180-347` — `F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty` — Representative movement parity must be tied to accepted source turns/steps, not screenshots with unknown party tuple. ok=`True`
- `DUNVIEW.C:8318-8611` — `F0128_DUNGEONVIEW_Draw_CPSF` — The viewport crop to compare is only promotable after F0128 composes G0296 for the known direction/X/Y tuple. ok=`True`
- `DRAWVIEW.C:709-858` — `F0097_DUNGEONVIEW_DrawViewport` — The original-side viewport artifact must be captured after the PC34 present seam, not at setup/BPLIST time. ok=`True`

## Existing evidence

- `pass360` `BLOCKED_PASS360_ORIGINAL_RUNTIME_TRUE_STOP_BLOCKER_NARROWED` — strict original FIRES F0128 -> F0097/VIDRV true-stop blocker ok=`True`
- `pass372` `PASS372_DM1_V1_MOVEMENT_RUNTIME_ROUTE_SOURCE_LOCKED` — Firestaff M11 movement route source-locked; not the active blocker ok=`True`

## Exact missing runtime artifacts

- `original_true_stop_transcript` — blocks: source-bound original runtime state identity; missing: A bounded FIRES debugger transcript proving F0128_DUNGEONVIEW_Draw_CPSF then F0097_DUNGEONVIEW_DrawViewport or VIDRV_09_BlitViewPort after controlled movement input.
- `semantic_original_full_frames` — blocks: movement/HUD/viewport overlay eligibility; missing: 320x200 original PC34 gameplay frames classified as party-control-ready movement/HUD/viewport states, with route labels and hashes.
- `pass70_original_viewport_crops` — blocks: viewport crop comparator; missing: 224x136 original viewport crops and manifest derived from semantically matched original frames.
- `paired_firestaff_capture_and_diff` — blocks: representative original-vs-Firestaff overlay review; missing: Same-scene Firestaff captures paired with the original crops plus diff masks/stats under parity-evidence/overlays/pass376.

## Exact next commands and required artifacts

1. `strict_original_true_stop_sequence`

   ```bash
   python3 tools/pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py --seconds 75 && python3 tools/verify_pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py && python3 tools/verify_pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing.py
   ```
   - must produce: parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/manifest.json with strict F0128 stop after running
   - must produce: same bounded transcript showing F0097_DUNGEONVIEW_DrawViewport or VIDRV_09_BlitViewPort after F0128

2. `semantic_original_runtime_capture`

   ```bash
   OUT_DIR=$PWD/verification-screens/pass376-original-route DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 DOSBOX=/usr/bin/dosbox DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 WAIT_BEFORE_INPUT_MS=3000 NEW_FILE_TIMEOUT_MS=6000 DM1_ORIGINAL_ROUTE_EVENTS="wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:2200 one wait:2500 shot:readiness_preflight wait:700 kp4 wait:900 shot:turn_left_after_vblank wait:700 kp6 wait:900 shot:turn_right_after_vblank wait:700 kp8 wait:1200 shot:forward_after_vblank wait:700 kp4 wait:900 shot:turn_left_2_after_vblank wait:700 kp6 wait:1200 shot:post_redraw_after_vblank" xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
   ```
   - must produce: verification-screens/pass376-original-route/imageNNNN-raw.png frames with 320x200 gameplay dimensions
   - must produce: route labels proving party-control-ready movement/HUD/viewport states, not menu/title/no-party duplicates

3. `original_viewport_crop_manifest`

   ```bash
   python3 tools/pass86_original_viewport_crop_manifest.py verification-screens/pass376-original-route --out-dir verification-screens/pass376-original-dm1-viewports
   ```
   - must produce: verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv
   - must produce: six 224x136 original viewport crops for pass70-compatible pairing

4. `paired_firestaff_overlay_compare`

   ```bash
   python3 tools/pass70_viewport_pair_compare.py --firestaff-dir verification-screens --original-dir verification-screens/pass376-original-dm1-viewports/viewport_224x136 --original-manifest verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv --out-dir parity-evidence/overlays/pass376 --plan-json parity-evidence/overlays/pass376/pass376_pairing_plan.json --run-diff
   ```
   - must produce: parity-evidence/overlays/pass376/pass376_pairing_plan.json
   - must produce: per-scene viewport diff masks/stats; these are eligibility artifacts, not parity claims until reviewed

## Non-claims

- No original-vs-Firestaff pixel parity is claimed.
- No new FIRES true-stop transcript is claimed.
- No semantically matched original runtime capture is claimed.

Manifest: `parity-evidence/verification/pass376_dm1_v1_original_overlay_parity_plan/manifest.json`
