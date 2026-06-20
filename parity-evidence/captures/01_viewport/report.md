# DM1 V1 Original Capture Pair Report

Pair index: **01_viewport**
Pair kind: **viewport**
Firestaff paired capture: (none)
pass80 classifier verdict: ****

## Captures

### 01_viewport_start

- Path: `/tmp/dm1_original_capture/01_viewport/01_viewport_start.png`
- SHA256: `0ce31ddfd29b525349ca8c545e6cc4e1fc5f7d699fe5370e4ca36c7c38c8f4f5`
- Size: 320x200
- pass80 classification: `entrance_menu`
- Notes: Frame after selector + entrance + ENTER; expected dungeon_gameplay

### 01_viewport_after_step

- Path: `/tmp/dm1_original_capture/01_viewport/01_viewport_after_step.png`
- SHA256: `60dac4b8430a0f47d4fcf075ecd81d6776f8d4a6624d22fdcdc35cad60524ad9`
- Size: 320x200
- pass80 classification: `entrance_menu`
- Notes: Frame after one C003_COMMAND_MOVE_FORWARD (KP5)

### 01_viewport_after_turn

- Path: `/tmp/dm1_original_capture/01_viewport/01_viewport_after_turn.png`
- SHA256: `bba0b02f0846e2cca8e948815db7e14fcb07209108afc6e7c18dc0df0deff69c`
- Size: 320x200
- pass80 classification: `entrance_menu`
- Notes: Frame after one C002_COMMAND_TURN_RIGHT (KP6)

## Notes

- 01_viewport_start: party-of-4 at start cell facing south; F0128_DUNGEONVIEW_Draw_CPSF should render the 3x3 viewport here.
- 01_viewport_after_step: party moved one cell south; ReDMCSB COMMAND.C:255 maps C003_COMMAND_MOVE_FORWARD to 0x000B/PC keypad-5.
- 01_viewport_after_turn: party turned right; ReDMCSB COMMAND.C:256 maps C002_COMMAND_TURN_RIGHT to 0x0095/PC keypad-6.

## Pass/Fail Verdict

**GAP_CLOSED**
