# Pass249 — DM1 V1 original runtime route unblock (N2)

Status: **PASS route/capture proof; debugger runtime-hit promotion still blocked**.

## What this proves

- Original DM1 PC 3.4 launches on N2 under `/usr/bin/dosbox` + `xvfb-run` from the canonical PC34 stage.
- The reusable route reaches live dungeon viewport frames. `pass80` classifies `image0001` and `image0002` as `dungeon_gameplay`.
- A posted gameplay input is accepted/applied visually: after `kp5`, raw SHA changes from `48ed3743ab6a...` to `47d61e2ae941...`; after a second `kp5`, the frame becomes `wall_closeup` (`ee7741746ea9...`). That is enough to unblock capture-route work, but not enough to promote C runtime hooks.
- 224x136 viewport crops were generated and hashed; only text manifests are committed here. Raw PNG/PPM files stay in `/tmp/dm1route_evidence`.

## Route used

```sh
OUT_DIR=/tmp/dm1route_evidence \
DM1_ORIGINAL_STAGE_DIR=<firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34 \
DM1_ORIGINAL_PROGRAM=DM -vv -sn -pk \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS=wait:7000 enter wait:1500 one wait:1500 click:276,140 wait:1500 one wait:1500 shot:gameplay_start kp5 wait:900 shot:after_kp5 kp5 wait:900 shot:after_kp5_second click:276,140 wait:900 shot:after_click_forward click:246,140 wait:900 shot:after_click_left i wait:900 shot:after_i \
DOSBOX=/usr/bin/dosbox \
xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

## Debugger/symbol status

- Actual loader bridge remains pass246: FIRES load segment `0733`.
- Candidate runtime formula: `runtime_cs = 0733 + static_cs`, `runtime_ip = static_ip`.
- Candidate breakpoints from `data/original_runtime/dm1_pc34_i34e_actual_loader_symbol_candidates.v1.json` / pass248: `22AF:06E9`, `1EA4:010D`, `1EA4:01AA`, `1859:0516`, `2AFF:110E`.
- `dosbox-debug` under tmux/xvfb accepts these BPs (pass247, rechecked manually), but noninteractive resume is still blocked: `tmux send-keys F5` writes `^[[15~` into the debugger prompt, and `C`/`RUN` are not continue commands. So this pass does **not** claim `verified_runtime_hit`.

## Source locks for promotion targets

- Command dequeue/accepted branch: `COMMAND.C:2045-2052`, `2075-2081`, `2095-2096`, `2118-2126`, `2150-2155`.
- Turn direction update: `CLIKMENU.C:142`, `167-173`.
- Move coordinate update/result path: `CLIKMENU.C:180`, `264-274`, `326-328`; `MOVESENS.C:316`, `442-443`, `494-495`, `556`.
- Viewport draw path: `GAMELOOP.C:35`, `78-90`, `215`; `DUNVIEW.C:8318-8355`, `8466-8542`.
- Globals to watch later: `TOWNSGLB.H:678-681`, `1371`, `1381-1383`.

## Files

- `manifest.json` — machine-readable summary
- `raw_manifest.tsv` — raw frame hashes/sizes
- `original_viewport_shot_labels.tsv` — route labels
- `original_viewport_224x136_manifest.tsv` — crop hashes
- `pass80_original_frame_classifier.json` — per-frame classifier metrics

## Next unblock

Use a terminal/PTY path that sends real debugger F5, or switch to a debugger interface with a textual continue command. Then hit the accepted runtime candidates and record command/coordinate globals before promoting `verified_runtime_hit`.
