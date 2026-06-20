# pass1052 DM1 V1 original turn-cycle viewport/wall capture

Status: PASS1052_ORIGINAL_TURNCYCLE_CAPTURE_CLEAN

## Scope

This pass captures a narrow original DM1 PC 3.4 DOSBox route that reaches the dungeon view and performs three left turns. It is evidence for original viewport/wall frame availability, not a full movement transcript and not an original-vs-Firestaff pixel parity claim.

## Command Shape

```sh
OUT_DIR=$PWD/verification-screens/pass1052-dm1-original-route-24h-turncycle \
DOSBOX=/opt/homebrew/bin/dosbox \
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
DM1_ORIGINAL_EXPECTED_SHOTS=4 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6500 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:2200 one wait:2500 kp5 wait:1200 shot:party_hud kp4 wait:1200 shot:left_1_wall kp4 wait:1200 shot:left_2_view kp4 wait:1200 shot:left_3_view' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run

python3 tools/pass80_original_frame_classifier.py \
  verification-screens/pass1052-dm1-original-route-24h-turncycle \
  --expected dungeon_gameplay,wall_closeup,dungeon_gameplay,wall_closeup \
  --fail-on-duplicates
```

## Result

- Raw screenshot health: PASS.
- Pass80 classifier: PASS.
- Captures: 4.
- Classes: 2 `dungeon_gameplay`, 2 `wall_closeup`.
- Duplicate raw frame count: 0.

| # | label | class | raw SHA256 |
|---|-------|-------|------------|
| 1 | party_hud | dungeon_gameplay | `40c678403d8f772822c1301bafa373adb0862915a8239d2bdb15f71fccf4b750` |
| 2 | left_1_wall | wall_closeup | `932d9d84e55fedb9ac836e733009f5902e44944d8cce70b5774afcca3a414b2b` |
| 3 | left_2_view | dungeon_gameplay | `966190b6ed4dd3d96b7b1302f2076b04cdc00f0551cb6b55dc4d6173bddb16fd` |
| 4 | left_3_view | wall_closeup | `278ba175878d0a06217dba37a504e949187fc9e821bb6614398b8104af41f8f9` |

## Artifacts

- `verification-screens/pass1052-dm1-original-route-24h-turncycle/raw_frame_health.json`
- `verification-screens/pass1052-dm1-original-route-24h-turncycle/pass80_original_frame_classifier.json`
- `verification-screens/pass1052-dm1-original-route-24h-turncycle/original_viewport_shot_labels.tsv`
- `verification-screens/pass1052-dm1-original-route-24h-turncycle/original_viewport_224x136_manifest.tsv`
- `verification-screens/pass1052-dm1-original-route-24h-turncycle/pass513_i34e_route_key_transcript_scaffold.json`
- `verification-screens/pass1052-dm1-original-route-24h-turncycle/image0001-raw.png`
- `verification-screens/pass1052-dm1-original-route-24h-turncycle/image0002-raw.png`
- `verification-screens/pass1052-dm1-original-route-24h-turncycle/image0003-raw.png`
- `verification-screens/pass1052-dm1-original-route-24h-turncycle/image0004-raw.png`

## Non-Claims

- The pass513 transcript remains scaffold-only. It records route-token-to-key intent and capture hashes, but it does not contain debugger-observed original PC/I34E `F0361`/`F0380` queue deltas, party tuple deltas, or `F0128`/`F0097` present-boundary values.
- No champion-panel or creature-chain route is proven by this pass.
- No original-vs-Firestaff pixel diff is claimed by this pass.
