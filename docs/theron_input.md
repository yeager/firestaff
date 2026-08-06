# Theron's Quest — Keyboard Controls

Theron's Quest (PC Engine) has no strafe — arrow Left/Right produce turns.

## PC Engine button mapping

| PCE Button | Keyboard (primary) | Keyboard (alt) |
|------------|-------------------|----------------|
| D-pad Up   | Up Arrow          | W              |
| D-pad Down | Down Arrow        | S              |
| D-pad Left | Left Arrow        | Home, Q        |
| D-pad Right| Right Arrow       | End, E         |
| Button I   | Return            | Space          |
| Button II  | Escape            |                |
| Run        | —                 |                |
| Select     | —                 |                |

## Startup menu

| Action              | Keys                        |
|---------------------|-----------------------------|
| Advance from title  | Return, Space               |
| Navigate menu       | Up/Down Arrow, W/S          |
| Select stage        | Return, Space               |
| Select/deselect hero| Return, Space               |
| Enter forcefield    | Return, Space               |
| Go back             | Escape                      |

## In-dungeon

| Action         | Keys                              |
|----------------|-----------------------------------|
| Move forward   | Up Arrow, W                       |
| Move backward  | Down Arrow, S                     |
| Turn left      | Left Arrow, Home, Q               |
| Turn right     | Right Arrow, End, E               |
| Wait (tick)    | Return, Space                     |

## Notes

- A/D are mapped to strafe in the engine but Theron ignores strafe input.
- Run and Select have no keyboard binding (no gameplay use currently).
- Gamepad input follows the SDL3 gamepad mapping (D-pad, A/B buttons).
- F5 = quick save, F9 = quick load, F12 = screenshot (engine-global).
# Firestaff runtime panel

Theron's Quest supports the shared **F10** graphics and cheats panel during
runtime. PRES, FILT and FX update the host presentation immediately. CH
contains only the verified shared cheat switch and slower/normal/faster live
scheduler. Navigate with arrows and Enter or mouse clicks; press Esc to close.
