# Pass376 — DM1 V1 original artifact command manifest

Status: `BLOCKED_PASS376_ORIGINAL_ARTIFACT_COMMAND_MANIFEST_READY`

## Decision

The overlay blocker is now an explicit artifact-command contract. No parity is claimed until the original true-stop transcript, labelled original frames, viewport crops, and paired diff artifacts all exist and pass their promotion rules.

## Source anchors

- `CLIKMENU.C:142-174,180-347` — Original frames must be tied to accepted source turns/steps, not unknown screenshots. ok=`True`
- `DUNVIEW.C:8318-8611` — The 224x136 viewport crop is promotable only after F0128 composes G0296 for a known direction/X/Y tuple. ok=`True`
- `DRAWVIEW.C:709-858` — Original-side capture must occur after the PC34 viewport-present seam, not setup/BPLIST echo text. ok=`True`

## Prior evidence

- `pass360` `BLOCKED_PASS360_ORIGINAL_RUNTIME_TRUE_STOP_BLOCKER_NARROWED` — strict original FIRES F0128 -> F0097/VIDRV true-stop blocker ok=`True`
- `pass372` `PASS372_DM1_V1_MOVEMENT_RUNTIME_ROUTE_SOURCE_LOCKED` — Firestaff M11 movement route source-locked; not the active blocker ok=`True`

## Exact commands and artifact paths

1. `strict_f0128_to_f0097_vidrv_true_stop_transcript` / `original_true_stop_transcript`

   ```bash
   python3 tools/pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py --seconds 75 && python3 tools/verify_pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py && python3 tools/verify_pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing.py
   ```

   Required outputs:
   - `parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/manifest.json`
   - `parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/pre_arm_before_route.clean.txt`
   - `parity-evidence/verification/pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing/manifest.json`
   - promotion rule: Promote only if the bounded transcript contains a strict post-Running stop at F0128_DUNGEONVIEW_Draw_CPSF followed by F0097_DUNGEONVIEW_DrawViewport or VIDRV_09_BlitViewPort; setup echo/BPLIST text is not evidence.

2. `labelled_original_320x200_frames` / `semantic_original_full_frames`

   ```bash
   OUT_DIR=$PWD/verification-screens/pass376-original-route DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 DOSBOX=/usr/bin/dosbox DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 WAIT_BEFORE_INPUT_MS=3000 NEW_FILE_TIMEOUT_MS=6000 DM1_ORIGINAL_ROUTE_EVENTS="wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:2200 one wait:2500 shot:readiness_preflight wait:700 kp4 wait:900 shot:turn_left_after_vblank wait:700 kp6 wait:900 shot:turn_right_after_vblank wait:700 kp8 wait:1200 shot:forward_after_vblank wait:700 kp4 wait:900 shot:turn_left_2_after_vblank wait:700 kp6 wait:1200 shot:post_redraw_after_vblank" xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
   ```

   Required outputs:
   - `verification-screens/pass376-original-route/image0001-raw.png through image0006-raw.png (320x200)`
   - `verification-screens/pass376-original-route/original_viewport_shot_labels.tsv`
   - `verification-screens/pass376-original-route/raw_manifest.tsv`
   - promotion rule: Promote only if pass80/pass86 classification shows gameplay/party-control-ready movement/HUD/viewport states, not menu/title/no-party duplicates.

3. `original_224x136_viewport_crops` / `pass70_original_viewport_crops`

   ```bash
   python3 tools/pass86_original_viewport_crop_manifest.py verification-screens/pass376-original-route --out-dir verification-screens/pass376-original-dm1-viewports
   ```

   Required outputs:
   - `verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv`
   - `verification-screens/pass376-original-dm1-viewports/viewport_224x136/01_ingame_start_original_viewport_224x136.png`
   - `verification-screens/pass376-original-dm1-viewports/viewport_224x136/02_ingame_turn_right_original_viewport_224x136.png`
   - `verification-screens/pass376-original-dm1-viewports/viewport_224x136/03_ingame_move_forward_original_viewport_224x136.png`
   - `verification-screens/pass376-original-dm1-viewports/viewport_224x136/04_ingame_spell_panel_original_viewport_224x136.png`
   - `verification-screens/pass376-original-dm1-viewports/viewport_224x136/05_ingame_after_cast_original_viewport_224x136.png`
   - `verification-screens/pass376-original-dm1-viewports/viewport_224x136/06_ingame_inventory_panel_original_viewport_224x136.png`
   - promotion rule: Promote only when all six crops are exactly 224x136 and manifest rows include kind/filename/width/height/bytes/sha256.

4. `paired_firestaff_original_diff_artifacts` / `paired_firestaff_capture_and_diff`

   ```bash
   python3 tools/pass70_viewport_pair_compare.py --firestaff-dir verification-screens --original-dir verification-screens/pass376-original-dm1-viewports/viewport_224x136 --original-manifest verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv --out-dir parity-evidence/overlays/pass376 --plan-json parity-evidence/overlays/pass376/pass376_pairing_plan.json --run-diff
   ```

   Required outputs:
   - `parity-evidence/overlays/pass376/pass376_pairing_plan.json`
   - `parity-evidence/overlays/pass376/*_viewport_original_vs_firestaff.mask.png`
   - `parity-evidence/overlays/pass376/*_viewport_original_vs_firestaff.stats.json`
   - promotion rule: Diff artifacts are eligibility/review inputs only; they do not by themselves claim pixel parity.

## Blockers

- The pass360 strict FIRES true-stop blocker remains active until the first command proves F0128 -> F0097/VIDRV in a bounded owned-PTY run.
- Existing pass94-era original frames/crops are historical inputs only; pass376 needs fresh labelled artifacts tied to this route contract before pairing.

## Non-claims

- No original-vs-Firestaff pixel parity is claimed.
- No new original FIRES F0128/F0097/VIDRV true stop is claimed.
- No semantically matched original runtime capture is claimed.
- No HUD/viewport overlay parity is claimed.

Manifest: `parity-evidence/verification/pass376_dm1_v1_original_artifact_command_manifest/manifest.json`
