# Pass435 — DM1 V1 semantic original-route readiness gate

Status: `BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY`

## Verdict

Original viewport crop tooling is ready, but semantic original-route promotion remains blocked.

## ReDMCSB WIP20210206 source audit

- `COMMAND.C:2045-2156` `F0380_COMMAND_ProcessQueue_CPSC` — ok=`True`; queued input must be observed through F0380 pop/load before turn commands branch to F0365 or movement commands branch to F0366
- `CLIKMENU.C:142-174` `F0365_COMMAND_ProcessTypes1To2_TurnParty` — ok=`True`; turn commands must be observed reaching F0365, where stop-wait is set and party direction mutates
- `CLIKMENU.C:180-347` `F0366_COMMAND_ProcessTypes3To6_MoveParty` — ok=`True`; move commands must be observed reaching F0366, where target square, move result, and movement cooldown are committed
- `DUNVIEW.C:8318-8611` `F0128_DUNGEONVIEW_Draw_CPSF` — ok=`True`; viewport crops are promotable only when F0128 composes G0296 for a known direction/X/Y tuple
- `DRAWVIEW.C:709-858` `F0097_DUNGEONVIEW_DrawViewport` — ok=`True`; the capture seam must be the PC34 viewport present path, not setup/menu echo

## Runtime semantic proof carried forward

- pass385 status: `BLOCKED_PASS385_F0365_F0366_COMMAND_DISPATCH_NOT_PROVEN`
- pass385 F0380 command queue hit: `True`
- pass385 F0365/F0366 command dispatch hit: `False`
- pass391 status: `PASS391_KEYBOARD_QUEUE_TO_F0380_DISPATCH_PROVEN`
- pass391 F0380 pop/load after queue write: `True`
- pass391 F0365/F0366 command dispatch observed: `True`
- G0321 stop-wait write observed: `True`
- later F0128 after stop-wait observed: `True`

## Artifact semantics

- pass434 readiness: `PASS_PASS434_ORIGINAL_VIEWPORT_CROP_READINESS`
- raw classifier sequence ok: `False`; classes: `['dungeon_gameplay', 'wall_closeup', 'dungeon_gameplay', 'dungeon_gameplay', 'wall_closeup', 'dungeon_gameplay']`
- raw duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 4, 'fbeb1b82cd096c15c2346f254d9b2b2e8c1a8d0b8d100ba1751c4230c51e3dde': 2}`
- crop manifest rows_all_224x136: `True`
- crop duplicate hashes: `{'701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c': 4, '1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81': 2}`

## Current route label/class/hash matrix

| # | route label | expected | actual | class ok | raw sha | crop sha |
|---|---|---|---|---|---|---|
| 01 | `readiness_preflight` | `dungeon_gameplay` | `dungeon_gameplay` | `True` | `48ed3743ab6a` | `701689e73fc0` |
| 02 | `turn_left_after_vblank` | `dungeon_gameplay` | `wall_closeup` | `False` | `fbeb1b82cd09` | `1e71ed879980` |
| 03 | `turn_right_after_vblank` | `dungeon_gameplay` | `dungeon_gameplay` | `True` | `48ed3743ab6a` | `701689e73fc0` |
| 04 | `forward_after_vblank` | `spell_panel` | `dungeon_gameplay` | `False` | `48ed3743ab6a` | `701689e73fc0` |
| 05 | `turn_left_2_after_vblank` | `dungeon_gameplay` | `wall_closeup` | `False` | `fbeb1b82cd09` | `1e71ed879980` |
| 06 | `post_redraw_after_vblank` | `inventory` | `dungeon_gameplay` | `False` | `48ed3743ab6a` | `701689e73fc0` |

## Duplicate hash groups

- raw frame duplicates by route index: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': ['01', '03', '04', '06'], 'fbeb1b82cd096c15c2346f254d9b2b2e8c1a8d0b8d100ba1751c4230c51e3dde': ['02', '05']}`
- viewport crop duplicates by route index: `{'701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c': ['01', '03', '04', '06'], '1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81': ['02', '05']}`

## Pass376 artifact quarantine

- quarantined: `True`
- reason: `raw classifier did not pass; raw classifier sequence does not match semantic promotion sequence; raw route repeats screenshot hashes; viewport crops repeat hashes`
- decision: quarantine as non-promotable historical evidence; do not use for semantic original-route readiness or pixel parity until a replacement six-state route is captured

## Next unblock command contract

Run the route capture, then strict crop manifest, then this gate again. These are actionability checks only; they do not claim pixel parity.

```bash
OUT_DIR=$PWD/verification-screens/pass376-original-route DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 DOSBOX=/usr/bin/dosbox DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 WAIT_BEFORE_INPUT_MS=3000 NEW_FILE_TIMEOUT_MS=6000 DM1_ORIGINAL_ROUTE_EVENTS="wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:2200 one wait:2500 shot:readiness_preflight wait:700 kp4 wait:900 shot:turn_left_after_vblank wait:700 kp6 wait:900 shot:turn_right_after_vblank wait:700 kp8 wait:1200 shot:forward_after_vblank wait:700 kp4 wait:900 shot:turn_left_2_after_vblank wait:700 kp6 wait:1200 shot:post_redraw_after_vblank" xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
python3 tools/pass86_original_viewport_crop_manifest.py verification-screens/pass376-original-route --out-dir verification-screens/pass376-original-dm1-viewports
python3 tools/verify_pass435_dm1_v1_semantic_original_route_readiness_gate.py
```

Promotion requires:
- six raw 320x200 frames classified as dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,spell_panel,dungeon_gameplay,inventory
- no duplicate raw frame hashes
- six 224x136 viewport crops with no duplicate crop hashes
- pass434 remains green and runtime evidence still proves F0380 -> F0365/F0366 -> G0321 -> later F0128

## Blockers

- pass376 original-route artifacts are quarantined as non-promotable duplicate/non-semantic evidence

## Promotion rule

Promote only when pass434 readiness is green, bounded original-runtime evidence proves F0380 pop/load plus F0365/F0366 command dispatch plus G0321 stop-wait write plus a later F0128 viewport draw, and six raw/cropped route states are non-duplicate and match dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,spell_panel,dungeon_gameplay,inventory.

This gate does not launch DOSBox and does not claim pixel parity.

Manifest: `parity-evidence/verification/pass435_dm1_v1_semantic_original_route_readiness_gate/manifest.json`
