# Theron's Quest — Keyboard Controls

Theron's Quest (PC Engine) has six movement-panel commands: turn left/right,
move forward/backward and step left/right. Firestaff's Theron route accepts
both SDL scancodes and keycodes, so W/A/S/D also work on macOS layouts or
input paths that do not populate scancodes.

## PC Engine button mapping

| PCE Button | Keyboard (primary) | Keyboard (alt) |
|------------|-------------------|----------------|
| D-pad Up   | Up Arrow          | W              |
| D-pad Down | Down Arrow        | S              |
| D-pad Left | Left Arrow, A     | Home, Q        |
| D-pad Right| Right Arrow, D    | End, E         |
| Button I   | Mouse 1, short touch | Z, Return, Space |
| Button II  | Mouse 2, long touch  | X, Escape         |
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
| Step left      | A                                 |
| Step right     | D                                 |
| Turn left      | Left Arrow, A, Home, Q            |
| Turn right     | Right Arrow, D, End, E            |
| Wait (tick)    | Return, Space                     |
| Next champion  | Tab                               |
| Pick up facing item | G                            |
| Drop last picked source item | P                    |
| Use facing door | U                                |

## Notes

- Arrow Left/Right and Home/End/Q/E rotate. A/D use the original lower-panel
  `$06/$04` side-step commands. The mapping is intentionally different even
  though A/D also appear as alternate PC Engine D-pad bindings above.
- Mouse button 1/2 are direct Button I/II bindings. A short touch emits
  Button I; a long touch emits Button II, including during startup.
- Run and Select have no keyboard binding (no gameplay use currently).
- Gamepad input follows the SDL3 gamepad mapping (D-pad, A/B buttons) where
  the selected input mode permits gamepad events.
- Pickup uses the authenticated Track 02 object occurrence and property row
  on source-backed levels. A successful pickup selects the exact source-backed
  inventory slot; P can return only that slot to the party's validated square.
  Changing champion clears the selection. I selects the next source-backed
  slot for the active champion, so a carried occurrence can be selected again
  after resume; compact item IDs without provenance are skipped. DROP without
  a selected source slot remains unavailable. Use operates an existing door in the facing cell;
  a locked source-level door remains closed until the original T900 key
  consumer is recovered.
- F12 = screenshot (engine-global).
- Theron currently has no usable Firestaff save command in the dungeon:
  `Ctrl+S` and `F5` report `THERON SAVE HANDOFF NOT READY`, and `F9` cannot
  restore a Theron runtime. This is intentional until the authenticated
  between-dungeon T080 save writer/restore handoff is connected. The older
  Firestaff `.tqsv` container remains fixture/tooling data and is neither
  advertised nor accepted by the production Continue route.

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
If punctuation still does not arrive, use the layout-stable fallback: `Z` is
Button I and `X` is Button II. The capture helper type-checks before posting
Quartz events, so a broken helper cannot silently make either binding look
unresponsive.

The PCE wire-state masks used by the replay helper follow Mednafen's device
vector order (`I`, `II`, `SELECT`, `RUN`, `UP`, `RIGHT`, `DOWN`, `LEFT`), not
the numeric `ConfigOrder` values shown in Mednafen's source.
# Firestaff runtime panel

Theron's Quest supports the shared **F10** graphics and cheats panel during
runtime. PRES, FILT and FX update the host presentation immediately. CH
contains only the verified shared cheat switch and slower/normal/faster live
scheduler. Navigate with arrows and Enter or mouse clicks; press Esc to close.
