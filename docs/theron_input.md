# Theron's Quest — Keyboard and Mouse Controls

Theron's Quest (PC Engine) has six movement-panel commands: turn left/right,
move forward/backward and step left/right. Firestaff's Theron route accepts
both SDL scancodes and keycodes, so W/A/S/D also work on macOS layouts or
input paths that do not populate scancodes.

Theron gameplay runs in Firestaff's native C/SDL runtime. External emulators
are research instruments only and do not run or replace the game in Firestaff.

## PC Engine button mapping

| Theron action | Keyboard (primary) | Keyboard (alt) |
|---------------|-------------------|----------------|
| D-pad Up      | Up Arrow          | W              |
| D-pad Down    | Down Arrow        | S              |
| D-pad Left    | Q                 | Home, keypad 4 |
| D-pad Right   | E                 | End, keypad 6  |
| Button I      | Z                 | Return, Space  |
| Button II     | X                 | Escape         |
| Run           | Return            |                |
| Select        | —                 |                |

The original PC Engine release has no mouse interface. Firestaff adds desktop
mouse controls as a host-side convenience. During
startup, left-click selects and right-click goes back. During dungeon play,
left-click activates one of the twelve visible control-panel actions. Holding
right mouse while pressing a direction key applies the manual's Button II
simplified-movement modifier; a right click by itself does not advance a tick.
Middle-click is inert. Moving the pointer highlights a panel control. Clicks
outside the panel currently do not trigger a game action. Direct clicks in the
dungeon view require a state-driven renderer and a Firestaff screen-to-world
mapping; they are not original Theron hit regions. Short and long touch
gestures send Button I and Button II, respectively.

## Startup menu

These are Firestaff's native-runtime bindings, not a PC Engine timing model.
On 2026-10-24, separate SDL key-down events advanced authentic JP media from
title through stage select and Soul Room into a loaded dungeon. A continuous
3–5 second hold has not been verified in Firestaff. The retail manual
documents Button I as the menu selection control but gives no multi-second
timing rule. See [DMWeb and Greatstone references](DMWEB_REFERENCE.md) and
the dated evidence in [TODO-theron.md](../TODO-theron.md).

| Action              | Keys                        |
|---------------------|-----------------------------|
| Advance from title  | Return, Space               |
| Navigate menu       | Up/Down Arrow, W/S          |
| Select stage        | Return, Space               |
| Select/deselect hero| Return, Space               |
| Enter forcefield    | Return, Space               |
| Go back             | Escape                      |

## In-dungeon

Turn input reports `BLOCKED` and leaves facing unchanged if the bounded
source-actuator event queue cannot reserve the events required by the turn.
The runtime receipt does not request a redraw in that case. This is a
fail-closed queue-capacity guarantee, not evidence about the original game's
actuator timing or retail queue capacity; see the 2026-10-24 entry in
[`DONE-theron.md`](../DONE-theron.md).

| Action         | Keys                              |
|----------------|-----------------------------------|
| Move forward   | Up Arrow, W, keypad 5/8           |
| Move backward  | Down Arrow, S, keypad 2           |
| Step left      | Left Arrow, A, keypad 1           |
| Step right     | Right Arrow, D, keypad 3          |
| Turn left      | Q, Home, keypad 4                 |
| Turn right     | E, End, keypad 6                  |
| Wait (tick)    | Space, Return                     |
| Next champion  | Tab                               |
| Pick up facing item | G                            |
| Select next source item | I                         |
| Drop last picked source item | P                    |
| Use facing object | U                               |

The native dungeon view also shows a twelve-button Firestaff mouse panel in
its lower-right corner. Its first two rows provide **TURN L**, **FORWARD**,
**TURN R**, **STEP L**, **BACK**, and **STEP R**. The remaining buttons are
**NEXT** champion, **PICKUP**, select the next source-backed **ITEM**, **DROP**
the selected source item, **USE** the object ahead, and **WAIT** one tick.
These buttons send the same native Theron input tokens as the corresponding
keyboard actions. Their captions use the active Theron translation catalog.
The panel scales with the game's presentation size. Japanese and Chinese
captions use ASCII romanizations because the native small font does not
contain CJK glyphs.
Hovering a control highlights it. Pointer movement requests a redraw only when
the highlighted control changes.

## Notes

- During loaded gameplay, arrows and A/D issue the source-backed side-step
  commands `$06/$04`; Q/E, Home/End and keypad 4/6 turn. Startup menu arrows
  retain their navigation behavior. This matches Firestaff's ordinary DM1
  dungeon keyboard layout while preserving Theron's native command tokens.
- SDL keyboard input routes the same supported loaded-dungeon commands as the
  native mouse panel: Tab cycles champion, G picks up, I selects the next
  source-backed item, P drops it, U uses the facing object, and Space/Return
  waits. These keys enter the Theron input facade directly; they do not rely
  on the boot-probe-only key mapper.
- The native mouse panel provides all six Theron movement actions and the
  implemented champion, item, use and wait actions. The retail manual's
  Button II chord is available as right-mouse plus Left/Right Arrow or A/D;
  it turns instead of side-stepping. A standalone right click does not advance
  a dungeon tick; in a loaded dungeon it is consumed only as the modifier,
  while Escape remains the desktop Back command. An SDL event-loop regression
  verifies that Button II plus Right turns, then that Right after release
  side-steps. A native SDL mouse sequence through the visible host panel also
  turns twice and advances across a source-backed tile, using authenticated
  USA Track 02. Existing US and Japanese
  Rev. 1 Track 02 runtime tests also exercise the panel's turn and movement
  commands against each region's loaded dungeon state. The Japanese test
  asserts both click receipts and the resulting party pose. These checks cover
  Firestaff's host panel, not original PCE mouse support or direct world-space
  clicks. The normal SDL-loop regression also uses the panel to pick up the
  authentic level-0 SANDALS, reselect that source-backed item and drop it;
  three consecutive TRV2 Debug runs verify the mapped pointer receipts and
  resulting party state against USA Track 02.
  The SDL-loop regression also clicks TURN L, STEP R then STEP L over a
  source-backed open passage, NEXT and WAIT; the return pose and mapped action
  receipts are checked against authentic USA Track 02.
- The host-panel bounds are Firestaff UI and do not reuse DM1's
  C007..C116 pointer tables. The original movement-panel captures identify
  six internal PCE cursor samples, not desktop hitboxes or host coordinates:
  turn-left `$7B/$8F`, forward `$8A/$8F`, turn-right `$98/$8F`, step-left
  `$7B/$A5`, backward `$8A/$A5`, and step-right `$98/$A5`. The samples
  informed the movement-control order only. Native state-driven Theron
  graphics and complete visual parity remain open; the overlay does not
  replace that renderer work.
  The turn and forward point captures are recorded in
  [`theron-original-turn-command-capture-2026-08-21.md`](source-lock/theron-original-turn-command-capture-2026-08-21.md)
  and
  [`theron-original-forward-command-capture-2026-08-21.md`](source-lock/theron-original-forward-command-capture-2026-08-21.md).
  Do not substitute DM1 hit regions or guessed coordinates.
- DM1-like mouse parity remains open: Theron's native dungeon view does not
  yet draw its original cursor, and clicks outside the Firestaff panel do not
  target the game world. The `$D515/$D588` interval candidate must be joined
  through the live pointer descriptor, hover result and Button-I command
  consumer before source-derived hit testing can be enabled. The viewport
  must also update from native game state for clicks to provide visible,
  end-to-end gameplay. See the current evidence and remaining capture gate in
  [`TODO-theron.md`](../TODO-theron.md).
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

## Historical external reference-capture notes

The following notes describe an external research setup only. It is not used
to run Theron in Firestaff.

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
