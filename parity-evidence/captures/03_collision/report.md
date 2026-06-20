# DM1 V1 Original Capture Pair Report

Pair index: **03_collision**
Pair kind: **collision**
Firestaff paired capture: (none)
pass80 classifier verdict: ****

## Captures

### 03_collision_before

- Path: `/tmp/dm1_original_capture/03_collision/03_collision_before.png`
- SHA256: `9164394fd354377f4413b407cb0bde127c279f93e0f287f8495c791b3f13fd0b`
- Size: 320x200
- pass80 classification: `entrance_menu`
- Notes: Frame before collision attempt

### 03_collision_attempt_1

- Path: `/tmp/dm1_original_capture/03_collision/03_collision_attempt_1.png`
- SHA256: `9164394fd354377f4413b407cb0bde127c279f93e0f287f8495c791b3f13fd0b`
- Size: 320x200
- pass80 classification: `entrance_menu`
- Notes: Collision attempt 1: C003_COMMAND_MOVE_FORWARD blocked by wall

### 03_collision_attempt_2

- Path: `/tmp/dm1_original_capture/03_collision/03_collision_attempt_2.png`
- SHA256: `9164394fd354377f4413b407cb0bde127c279f93e0f287f8495c791b3f13fd0b`
- Size: 320x200
- pass80 classification: `entrance_menu`
- Notes: Collision attempt 2: C003_COMMAND_MOVE_FORWARD blocked by wall

### 03_collision_attempt_3

- Path: `/tmp/dm1_original_capture/03_collision/03_collision_attempt_3.png`
- SHA256: `9164394fd354377f4413b407cb0bde127c279f93e0f287f8495c791b3f13fd0b`
- Size: 320x200
- pass80 classification: `entrance_menu`
- Notes: Collision attempt 3: C003_COMMAND_MOVE_FORWARD blocked by wall

### 03_collision_attempt_4

- Path: `/tmp/dm1_original_capture/03_collision/03_collision_attempt_4.png`
- SHA256: `9164394fd354377f4413b407cb0bde127c279f93e0f287f8495c791b3f13fd0b`
- Size: 320x200
- pass80 classification: `entrance_menu`
- Notes: Collision attempt 4: C003_COMMAND_MOVE_FORWARD blocked by wall

## Notes

- 03_collision: the C003_COMMAND_MOVE_FORWARD command lands in the queue via COMMAND.C F0361/F0380; the dungeon collision layer in DUNGEON.C rejects moves into a wall cell, so the viewport does not change.

## Pass/Fail Verdict

**GAP_CLOSED (with collision/blocked steps)**
- 4 duplicate SHA(s) detected (expected for collision/creature/wall-blocked pairs)

### SHA distribution

- `9164394fd354`: 5 capture(s)
