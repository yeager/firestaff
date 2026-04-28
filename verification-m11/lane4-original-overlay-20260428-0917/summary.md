# Lane 4 — original overlay/capture route evidence (N2)

Scope: original DOSBox/capture/overlay readiness only. No source/HUD/viewport/V2 changes and no promotion of original captures as parity evidence.

## Commands run

```sh
git status --short --branch
git log --oneline -5
python3 tools/pass84_original_overlay_readiness_probe.py > verification-m11/lane4-original-overlay-20260428-0917/pass84_before.json
OUT_DIR=$PWD/verification-m11/lane4-original-overlay-20260428-0917/prepare \
  DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 \
  DOSBOX=/usr/bin/dosbox \
  scripts/dosbox_dm1_original_viewport_reference_capture.sh --prepare
OUT_DIR=$PWD/verification-m11/lane4-original-overlay-20260428-0917/dry-run \
  DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 \
  DOSBOX=/usr/bin/dosbox \
  DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 shot:party_hud right wait:300 shot up wait:300 shot wait:300 shot:spell_panel wait:300 shot wait:300 shot:inventory_panel' \
  DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
  scripts/dosbox_dm1_original_viewport_reference_capture.sh --dry-run
OUT_DIR=$PWD/verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic \
  DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 \
  DOSBOX=/usr/bin/dosbox \
  DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
  DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
  WAIT_BEFORE_INPUT_MS=5000 \
  NEW_FILE_TIMEOUT_MS=6000 \
  DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 shot:title enter wait:1200 shot:pre_enter_menu click:260,50 wait:1200 shot:after_enter_click click:276,140 wait:600 shot:forward_1 click:276,140 wait:600 shot:forward_2 click:246,140 wait:600 shot:left_turn_probe' \
  xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
python3 tools/pass78_original_route_attempt_audit.py verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic \
  --out-json verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/pass78_audit.json
python3 tools/pass80_original_frame_classifier.py verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic \
  --expected pass94-diagnostic --fail-on-duplicates \
  --out-json verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/pass80_classifier.json \
  --out-md verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/pass80_classifier.md
```

## Results

- PASS: N2 has required capture tooling: `/usr/bin/dosbox`, `xvfb-run`, `xdotool`, `7z`, ImageMagick `convert`, Python Pillow.
- PASS: `--prepare` and route-shape `--dry-run` succeed with the external canonical DM1 stage at `$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34`.
- PASS: pass94 diagnostic capture now produces six raw `320x200` DOSBox graphics screenshots on N2; `pass78_audit.json` has `all_gameplay_320x200=true`.
- FAIL/BLOCKED: pass94 semantic classifier does **not** pass. `pass80_classifier.json` class counts are `entrance_menu:3`, `graphics_320x200_unclassified:1`, `wall_closeup:2`; expected `title_or_menu, entrance_menu, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay` was not met.
- BLOCKER: `click:260,50` did not reliably leave the entrance/menu state by shot 03, movement clicks lead to unsafe entrance/wall-closeup frames, and duplicate raw frames are present. Do not promote these screenshots as original overlay references.
- BLOCKER: default pass84 readiness remains false before promotion because Firestaff PPMs/default original raws/masks are absent in this fresh worktree and the semantic original route is not locked.

## Evidence files

- `pass84_before.json` — existing overlay readiness blockers.
- `prepare/` — generated DOSBox config and route helpers.
- `dry-run/` + `dry-run.log` — expected six-shot semantic route shape validates syntactically.
- `pass94-diagnostic/` — raw screenshots, normalized crops/manifests, key log, DOSBox log, pass78/pass80 classifier evidence.
