# Theron's Quest — Keyboard Controls

Theron's Quest (PC Engine) has no strafe — arrow Left/Right produce turns.

## PC Engine button mapping

| PCE Button | Keyboard (primary) | Keyboard (alt) |
|------------|-------------------|----------------|
| D-pad Up   | Up Arrow          | W              |
| D-pad Down | Down Arrow        | S              |
| D-pad Left | Left Arrow        | Home, Q        |
| D-pad Right| Right Arrow       | End, E         |
| Button I   | Z                 | Return, Space  |
| Button II  | X                 | Escape         |
| Run        | Return            |                |
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

## Mednafen macOS capture profile

The recommended macOS profile assigns PCE Button I to `Z` and Button II to `X`.
This avoids keyboard-layout-dependent punctuation. Comma and period are valid
only when the active `mednafen.cfg` explicitly assigns their SDL scancodes to
`gamepad.i` and `gamepad.ii`; they are not universal aliases. The Firestaff
capture helper resolves the host key from that profile instead of assuming a
character key.

Mednafen's emulated keyboard input must also be grabbed before these keys can
reach the PCE pad. Most Mac keyboards have no `Menu` key, so the Theron macOS
profile binds `command.toggle_grab` to SDL scancode `10`: press
`Ctrl+Shift+G` once after the game window is focused. With grabbing enabled,
comma is SDL scancode `54` and period is `55`; they work as Button I/II only
when the profile assigns those scancodes to the corresponding PCE buttons.

The PCE wire-state masks used by the replay helper follow Mednafen's device
vector order (`I`, `II`, `SELECT`, `RUN`, `UP`, `RIGHT`, `DOWN`, `LEFT`), not
the numeric `ConfigOrder` values shown in Mednafen's source.
# Firestaff runtime panel

Theron's Quest supports the shared **F10** graphics and cheats panel during
runtime. PRES, FILT and FX update the host presentation immediately. CH
contains only the verified shared cheat switch and slower/normal/faster live
scheduler. Navigate with arrows and Enter or mouse clicks; press Esc to close.
