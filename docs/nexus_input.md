# Nexus V1 — Input System Audit

## Sources
- `src/engine/firestaff_input.c` (input queue, keyboard mapping)
- `src/engine/firestaff_touch.c` (touch-to-mouse abstraction)
- `src/engine/firestaff_controller.c` (gamepad/controller support)
- `docs/dm2_input.md` (DM2 input system reference)
- `src/frontend/entrance_keyboard_routes_pc34_compat.c` (DM1 keyboard routing)
- `src/frontend/entrance_mouse_routes_pc34_compat.c` (DM1 mouse routing)

## Overview

Nexus V1 input is handled by the Firestaff input layer — the same system used
for DM1, M11, and M12. The Nexus engine has no input handling code;
keyboard/mouse/gamepad events are processed by firestaff_input.c and routed
to game commands (movement, menu, action) which the Nexus engine interprets.

On Sega Saturn, input comes from the SFC-style controller (D-pad + 6 face
buttons + 2 shoulder triggers). On PC/Linux, this is emulated via SDL2.

## Input Command System

FS_Command enum (src/engine/firestaff_input.h):

| Command | Key (Arrow) | Key (WASD) | Action |
|---------|-------------|------------|--------|
| FS_CMD_MOVE_FORWARD | Up / W | W | Move forward |
| FS_CMD_MOVE_BACKWARD | Down / S | S | Move backward |
| FS_CMD_TURN_LEFT | Left | A | Turn left 90 deg |
| FS_CMD_TURN_RIGHT | Right | D | Turn right 90 deg |
| FS_CMD_STRAFE_LEFT | — | A | Strafe left (if WASD enabled) |
| FS_CMD_STRAFE_RIGHT | — | D | Strafe right (if WASD enabled) |
| FS_CMD_MENU | Escape | Escape | Open ESC menu |
| FS_CMD_ACTION | Space | Space | Confirm / attack |
| FS_CMD_INVENTORY | Tab | Tab | Toggle inventory |

Input queue: FS_InputQueue — FIFO command queue, size FS_INPUT_QUEUE_SIZE (8).

## Keyboard Input Mapping

fs_input_key_to_command() (firestaff_input.c):
- SDL scancodes: Up=82, Down=81, Left=80, Right=79
- WASD: W=26, A=4, S=22, D=7

## Mouse Input (DM1/M11/M12 reference)

DM1 PC 3.4 mouse handling (entrance_mouse_routes_pc34_compat.c):
- Click zones defined in CEDTDATA.C (mouse input records per dialog type)
- G2259_as_MouseInput_Dialog2Choices[3] — 2-choice dialog buttons
- G2260_as_MouseInput_Dialog3ChoicesA[4] — 3-choice dialog
- G2285_as_FilePickerDialogButtons[5] — file picker buttons

## Touch Input

firestaff_touch.c maps SDL touch/finger events to synthesized mouse events
through M11_GameView_HandlePointerButton(). No duplicate click logic.

Gestures:
- Single tap: Left click (move / confirm)
- Long press (>500ms): Right click (inventory panel)
- Horizontal swipe: Turn left/right
- Vertical swipe: Move forward/backward

Swipe threshold: FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX = 40 pixels.

## Controller / Gamepad Input

firestaff_controller.c — SDL gamepad mapping for Saturn SFC pad:
- D-pad: 4-directional movement
- A: Action / confirm
- B: Cancel / back
- X: Inventory
- Y: Menu (ESC)
- L/R: Strafe (if enabled)
- Start: Pause

## DM1 vs Nexus Input Comparison

| Feature | DM1 | Nexus V1 (Firestaff) |
|---------|-----|-----------------------|
| Keyboard | BIOS INT 16h | SDL keyboard API |
| Mouse | INT 33h | SDL mouse |
| Gamepad | None | SDL gamepad |
| Touch | None | SDL touch |
| Event system | Immediate | FIFO command queue |

## Nexus-Specific Input Notes

### Saturn Controller (SFC Pad)
Native input device. Button mapping:
- D-pad: 4-directional movement
- A/B/X/Y: Face buttons (A=confirm, B=cancel, X=inventory, Y=menu)
- L/R: Left/right shoulder buttons
- Start: Pause

### Input in Nexus Engine
nexus_v1_engine.c has no input handling. It receives commands via the
Firestaff command queue. nexus_v1_tick() is called each frame by the game
loop — input is processed before nexus_v1_tick().

### Menu Navigation
No menu system exists yet for Nexus. When implemented, will need:
- Title screen: D-pad / arrow keys to navigate, A/Start to confirm
- Champion select: D-pad to scroll roster, A to select
- In-game menu: ESC/Y to open, D-pad to navigate, B to cancel

## Whats Implemented vs Whats Missing

Implemented: Keyboard command mapping, input queue FIFO, touch-to-mouse
gesture system, SDL gamepad mapping, ESC/Tab/Space action keys, WASD toggle.

Not yet implemented: Nexus menu input routing, mouse click-to-square
routing in 3D viewport, champion portrait click, inventory drag-and-drop,
key binding customization, pause menu (ESC routed but no menu exists).

## Next Steps
1. Implement mouse click-to-square routing in 3D viewport
2. Implement Nexus ESC menu with D-pad navigation
3. Add key binding customization via config_m12.c settings
4. Implement champion portrait click (party member selection)
5. Add pause menu binding (Start button on gamepad)
