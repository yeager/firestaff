# Pass304 - DM1 V1 original PC34 viewport capture blocker manifest

Status: BLOCKED_ORIGINAL_PC34_VIEWPORT_CAPTURE_NOT_ROUTE_PROVEN

This pass does not claim pixel parity. It turns the pass127/pass300 wall render-plan seam into a deterministic original-capture contract for wall-comparator promotion.

## Required crop

- Full original frame: 320x200
- Viewport crop: 224x136+0+33 (script crop (0,33,224,169))
- PPM SHA-256 remains the comparator/hash source of truth; PNG is only a viewable companion.

## Required route states

| snapshot | party tuple | batch | route from fresh gameplay | wall events |
| --- | --- | --- | --- | ---: |
| start_south | map 0, x 1, y 3, dir 2 | A | (fresh gameplay start) | 8 |
| turn_right_west | map 0, x 1, y 3, dir 3 | A | right | 13 |
| move_forward_west | map 0, x 0, y 3, dir 3 | A | right up | 14 |
| turn_left_east | map 0, x 1, y 3, dir 1 | B | left | 8 |
| blocked_forward_south_wall | map 0, x 1, y 3, dir 2 | C | up | 8 |

## Existing crop audit

- Usable for promotion: False
- Reason: tracked original viewport manifests do not contain route labels for the pass127/pass304 required snapshot names; several crops are duplicate no-party/diagnostic hashes, so they cannot be bound to the required party tuple/F0128 state.
- Existing unique crop hashes scanned: 6

## Required source assets

- DM.EXE: 4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4 (11471 bytes, <firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34/DM.EXE)
- DUNGEON.DAT: d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85 (33357 bytes, <firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT)
- GRAPHICS.DAT: 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e (363417 bytes, <firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT)
- TITLE: adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745 (12002 bytes, <firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34/TITLE)

## GRAPHICS.DAT decode blocker

- Already manifested by pass302: [78, 79, 107]
- Still required by this wall render plan: [93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107]
- Blocker: pass302 proves only 78/79/107; wall comparator promotion needs decoded bitmap/palette byte manifests for every listed wall-set graphic index used by pass304 snapshots, without broad dumps.

## Next capture commands

Known blocker: Current script requires exactly six shots and has no reset/state-restore marker. The pass127 comparator set branches from the fresh start tuple, so exact promotion needs three independent batches or a script enhancement that supports multiple fresh-run batches with per-shot labels.

### batchA_start_right_forward

```sh
OUT_DIR=$PWD/verification-screens/pass304-original-pc34-wall-comparator-batch-A \
DM1_ORIGINAL_STAGE_DIR=<firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34 \
DM1_ORIGINAL_PROGRAM="DM -vv -sn -pk" \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS="wait:7000 shot:start_south right wait:600 shot:turn_right_west up wait:600 shot:move_forward_west wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion" \
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
DM1_ORIGINAL_ROUTE_EVENTS="wait:7000 shot:start_south left wait:600 shot:turn_left_east wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion" \
DOSBOX=/usr/bin/dosbox xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

### batchC_start_blocked_forward

```sh
OUT_DIR=$PWD/verification-screens/pass304-original-pc34-wall-comparator-batch-C \
DM1_ORIGINAL_STAGE_DIR=<firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34 \
DM1_ORIGINAL_PROGRAM="DM -vv -sn -pk" \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS="wait:7000 shot:start_south up wait:600 shot:blocked_forward_south_wall wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion wait:600 shot:padding_not_for_promotion" \
DOSBOX=/usr/bin/dosbox xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

## Verification

```sh
python3 -m py_compile tools/verify_pass304_dm1_v1_original_viewport_capture_blocker_manifest.py
python3 tools/verify_pass304_dm1_v1_original_viewport_capture_blocker_manifest.py
python3 -m json.tool parity-evidence/verification/pass304_dm1_v1_original_viewport_capture_blocker_manifest.json >/dev/null
```
