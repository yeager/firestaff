# Pass1055 DM1 V1 original closed-door collision capture

Status: `PASS1055_ORIGINAL_CLOSED_DOOR_STASIS_CAPTURED`

This pass captures a deterministic DM1 PC 3.4 DOSBox route to a closed Hall-area
door and records the original runtime response after two interaction attempts:

1. viewport click at the closed door: `click:112,100`
2. forward keypress against the same closed door: `kp5`

The three closed-door frames are byte-identical at both the raw 320x200 frame
level and the 224x136 viewport-crop level. This is original runtime evidence that
the attempted action/forward step leaves the party view unchanged at this closed
door.

Firestaff now has a matching movement/collision probe for the same key sequence:
`firestaff_dm1_v1_pass1055_closed_door_pair_probe`.

## Inputs

- Canonical game: Dungeon Master PC DOS English v3.4
- DUNGEON.DAT SHA256: `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
- GRAPHICS.DAT SHA256: `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`
- Harness: `scripts/dosbox_dm1_original_viewport_reference_capture.sh`
- Artifact directory: `verification-screens/pass1055-dm1-original-closed-door-collision/`

## Route

The route enters the dungeon, follows a map-driven Hall path to a closed door,
then records:

| Capture label | Route token |
|---|---|
| `start` | `shot:start` |
| `door_before` | `shot:door_before` |
| `after_viewport_click` | `click:112,100`, `shot:after_viewport_click` |
| `after_kp5` | `kp5`, `shot:after_kp5` |

The full key transcript is tracked as
`verification-screens/pass1055-dm1-original-closed-door-collision/original-viewpoint-route-keys.log`.

## Raw-frame results

| Frame | Label | Class | SHA256 |
|---:|---|---|---|
| 1 | `start` | `dungeon_gameplay` | `40c678403d8f772822c1301bafa373adb0862915a8239d2bdb15f71fccf4b750` |
| 2 | `door_before` | `wall_closeup` | `a0d3a9cdbddc310e3ef195c9c7719508a5141fbd66e1acb6a8dbe4b14ebc0dd6` |
| 3 | `after_viewport_click` | `wall_closeup` | `a0d3a9cdbddc310e3ef195c9c7719508a5141fbd66e1acb6a8dbe4b14ebc0dd6` |
| 4 | `after_kp5` | `wall_closeup` | `a0d3a9cdbddc310e3ef195c9c7719508a5141fbd66e1acb6a8dbe4b14ebc0dd6` |

## Viewport-crop results

| Crop | Label | SHA256 |
|---|---|---|
| `01_start_original_viewport_224x136.ppm` | `start` | `3cffaf384e041c349ea3c2f0d9d4b27be86b41a1c713e7a8698cc3eefe23ffa2` |
| `02_door_before_original_viewport_224x136.ppm` | `door_before` | `93a07d28805f4a0e554607899406b6d706c88be920415940d2453069e673a5f6` |
| `03_after_viewport_click_original_viewport_224x136.ppm` | `after_viewport_click` | `93a07d28805f4a0e554607899406b6d706c88be920415940d2453069e673a5f6` |
| `04_after_kp5_original_viewport_224x136.ppm` | `after_kp5` | `93a07d28805f4a0e554607899406b6d706c88be920415940d2453069e673a5f6` |

## Promoted evidence

- The original closed-door frame is stable across `door_before`,
  `after_viewport_click`, and `after_kp5`.
- Firestaff replays the same movement key sequence to `map=0 x=6 y=9 dir=3`,
  identifies the target square `(5,9)` as door square `0x94`, and blocks the
  following forward command with `MOVE_BLOCKED_DOOR` while keeping the party at
  `(0,6,9,3)`.
- This narrows the collision evidence gap from no original runtime capture to one
  original closed-door stasis capture with a Firestaff-side semantic pair.
- The route also documents the current blocker for the attempted Hall-to-map1
  creature route: this path reaches a closed door, and the tested viewport click
  does not open it.

## Firestaff-side pair

Command:

```sh
./build/firestaff_dm1_v1_pass1055_closed_door_pair_probe
```

Observed output:

```text
[before_closed_door] map=0 x=6 y=9 dir=3 target=(5,9) square=0x94 type=4 passable=0
[after_forward_into_closed_door] resultCode=2 blocked=1 anyMove=0 pos=(0,6,9,3)
result=PASS
```

## Machine-check gate

`pass1055_dm1_v1_original_closed_door_collision_capture` is now registered in
CTest. It verifies the original raw-frame stasis, viewport-crop stasis,
pass513 scaffold boundary, and Firestaff-side semantic pair probe, then writes
`parity-evidence/verification/pass1055_dm1_v1_original_closed_door_collision_capture/manifest.json`.

## Non-claims

- This is not a Firestaff-vs-original pixel comparison.
- This is not a complete collision transcript for walls, fake walls, doors, and
  door-state changes.
- This is not a creature capture; no monster appears in this route.
- The pass80 classifier reports duplicate frames by design here. For this pass,
  the duplicate SHA is the measured stasis signal, not a failed route artifact.
