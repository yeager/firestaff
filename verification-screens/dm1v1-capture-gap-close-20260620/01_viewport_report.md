# DM1 V1 Original Capture Pair Report

Pair index: **01_viewport**
Pair kind: **viewport**
Firestaff paired capture: (none)
pass80 classifier verdict: ****

## Captures

### 01_viewport_start

- Path: `/tmp/dm1_original_capture/01_viewport/01_viewport_start.png`
- SHA256: `8f62254638b4f69601158bca1cd505dc1bd344f8e55a11cafd516417e8b2a87c`
- Size: 320x200
- pass80 classification: `entrance_menu`
- Notes: Frame after selector + entrance + ENTER; expected dungeon_gameplay

### 01_viewport_after_step

- Path: `/tmp/dm1_original_capture/01_viewport/01_viewport_after_step.png`
- SHA256: `0e6eeb39bbbb314397882b796aa2c16c22a8e1723b13a82330705e2699b7aeb4`
- Size: 320x200
- pass80 classification: `entrance_menu`
- Notes: Frame after one C003_COMMAND_MOVE_FORWARD (KP5)

### 01_viewport_after_turn

- Path: `/tmp/dm1_original_capture/01_viewport/01_viewport_after_turn.png`
- SHA256: `8c43b579576568cac9754dc4b26f3c73a073af3376c82c0d75341123197d029a`
- Size: 320x200
- pass80 classification: `entrance_menu`
- Notes: Frame after one C002_COMMAND_TURN_RIGHT (KP6)

## Notes

- 01_viewport_start: party-of-4 at start cell facing south; F0128_DUNGEONVIEW_Draw_CPSF should render the 3x3 viewport here.
- 01_viewport_after_step: party moved one cell south; ReDMCSB COMMAND.C:255 maps C003_COMMAND_MOVE_FORWARD to 0x000B/PC keypad-5.
- 01_viewport_after_turn: party turned right; ReDMCSB COMMAND.C:256 maps C002_COMMAND_TURN_RIGHT to 0x0095/PC keypad-6.

## Pass/Fail Verdict

**GAP_CLOSED**

### SHA distribution

- `8f62254638b4`: 1 capture(s)
- `0e6eeb39bbbb`: 1 capture(s)
- `8c43b5795765`: 1 capture(s)
