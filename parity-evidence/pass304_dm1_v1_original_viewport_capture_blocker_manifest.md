# Pass304 - DM1 V1 original PC34 viewport capture blocker manifest

Status: BLOCKED_ORIGINAL_PC34_VIEWPORT_CAPTURE_NOT_ROUTE_PROVEN

This pass does not claim pixel parity. It now reflects the current source/render-plan snapshot names: the old `move_forward_west` and `blocked_forward_south_wall` captures are stale review evidence, while `forward_west_blocked` and `forward_south_corridor` need fresh route-proven PC34 captures before comparator promotion.

## Required crop

- Full original frame: 320x200
- Viewport crop: 224x136+0+33 (script crop (0,33,224,169))
- PPM SHA-256 remains the comparator/hash source of truth; PNG is only a viewable companion.

## Required route states

| snapshot | party tuple | batch | route from fresh gameplay | wall events |
| --- | --- | --- | --- | ---: |
| start_south | map 0, x 1, y 3, dir 2 | A | (fresh gameplay start) | 11 |
| turn_right_west | map 0, x 1, y 3, dir 3 | A | right | 13 |
| forward_west_blocked | map 0, x 1, y 3, dir 3 | A | right up | 13 |
| turn_left_east | map 0, x 1, y 3, dir 1 | B | left | 9 |
| forward_south_corridor | map 0, x 1, y 4, dir 2 | C | up | 9 |

## Existing crop/state audit

- Required route label coverage: False (`forward_west_blocked` and `forward_south_corridor` are not present in the existing original crop label manifests).
- Capture execution support: pass308 remains useful historical execution evidence, but its old `move_forward_west` / `blocked_forward_south_wall` labels do not satisfy the current render-plan snapshot names.
- State-oracle support: pass312 `PASS_STATE_ORACLE_SOURCE_RUNTIME_BOUND`; command input -> queue write -> party tuple mutation -> F0128 viewport draw consumption is source/runtime-bound.
- Usable as comparator inputs: blocked until fresh PC34 captures contain route labels for `forward_west_blocked` and `forward_south_corridor`; duplicate hashes remain semantic evidence only and are not pixel parity.
- Existing unique crop hashes scanned: 7


## ReDMCSB source audit anchors

Primary source root audited before verifier changes: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source`. Relevant state/capture binding anchors are inherited from pass312 and recorded here for pass304 traceability:

- `GAMELOOP.C` `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)`: lines 80-90; command queue processing call: lines 214-215.
- `COMMAND.C` `F0380_COMMAND_ProcessQueue_CPSC`: lines 2045-2126; turn/move dispatch: lines 2150-2156.
- `CLIKMENU.C` `F0365_COMMAND_ProcessTypes1To2_TurnParty`: lines 142-173; `F0366_COMMAND_ProcessTypes3To6_MoveParty`: lines 180-329.
- `CHAMPION.C` `F0284_CHAMPION_SetPartyDirection`: lines 93-130.
- `MOVESENS.C` party destination writes/teleporter/falling draw cases: lines 438-443, 493-517, 550-556.
- `DUNVIEW.C` `F0128_DUNGEONVIEW_Draw_CPSF`: signature lines 8318-8324, tuple view calculation lines 8356-8542, viewport present lines 8604-8610.

## Required source assets

- DM.EXE: 4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4 (11471 bytes, <firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34/DM.EXE)
- DUNGEON.DAT: d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85 (33357 bytes, <firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT)
- GRAPHICS.DAT: 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e (363417 bytes, <firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT)
- TITLE: adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745 (12002 bytes, <firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34/TITLE)

## Current blocker

- Primary blocker: required original capture labels are missing for `forward_west_blocked` and `forward_south_corridor`; the current manifests only contain stale labels (`move_forward_west`, `blocked_forward_south_wall`).
- Secondary blocker after recapture: GRAPHICS.DAT wall-set decode coverage must remain complete for the current render plan.

## GRAPHICS.DAT decode status

- Already manifested by pass302/pass305: [78, 79, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107]
- Required by this wall render plan: [93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107]
- Exact missing decoded wall-set records: []
- Blocker: none for decoded wall-set records in the current bounded pass305 manifest; comparator promotion is still blocked by missing fresh original capture labels above.

## Next capture commands

No additional original viewport capture executable is missing for pass304. The three existing batch manifests provide the required labels; rerun commands remain below only for reproducibility.

### batchA_start_right_forward_blocked

```sh
OUT_DIR=$PWD/verification-screens/pass304-original-pc34-wall-comparator-batch-A \
DM1_ORIGINAL_STAGE_DIR=<firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34 \
DM1_ORIGINAL_PROGRAM="DM -vv -sn -pk" \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS="wait:7000 enter wait:1500 click:260,50 wait:1500 click:276,140 wait:3000 shot:start_south kp6 wait:1200 shot:turn_right_west kp8 wait:1200 shot:forward_west_blocked wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion" \
DOSBOX=/usr/bin/dosbox xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

### batchB_start_left

```sh
OUT_DIR=$PWD/verification-screens/pass304-original-pc34-wall-comparator-batch-B \
DM1_ORIGINAL_STAGE_DIR=<firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34 \
DM1_ORIGINAL_PROGRAM="DM -vv -sn -pk" \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS="wait:7000 enter wait:1500 click:260,50 wait:1500 click:276,140 wait:3000 shot:start_south kp4 wait:1200 shot:turn_left_east wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion" \
DOSBOX=/usr/bin/dosbox xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

### batchC_start_forward_south_corridor

```sh
OUT_DIR=$PWD/verification-screens/pass304-original-pc34-wall-comparator-batch-C \
DM1_ORIGINAL_STAGE_DIR=<firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34 \
DM1_ORIGINAL_PROGRAM="DM -vv -sn -pk" \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS="wait:7000 enter wait:1500 click:260,50 wait:1500 click:276,140 wait:3000 shot:start_south kp8 wait:1200 shot:forward_south_corridor wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion" \
DOSBOX=/usr/bin/dosbox xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

## Verification

```sh
python3 -m py_compile tools/verify_pass304_dm1_v1_original_viewport_capture_blocker_manifest.py
python3 tools/verify_pass304_dm1_v1_original_viewport_capture_blocker_manifest.py
python3 -m json.tool parity-evidence/verification/pass304_dm1_v1_original_viewport_capture_blocker_manifest.json >/dev/null
```
