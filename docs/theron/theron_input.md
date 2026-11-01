# Theron's Quest — Keyboard and Mouse Controls

**Status reviewed 2026-11-01.** These controls describe Firestaff's native
desktop mapping. SDL regression coverage verifies command dispatch and
bounded US pickup/reselect/drop; it does not establish full retail input
semantics. On 2026-10-31, the macOS SDL3 Debug build passed three runs each of
the authentic US and Japanese Rev. 1 input suites. This still does not verify
world-space clicks or visual parity. See the
[capture-readiness summary](THERON_CAPTURE_READINESS.md).

Theron's Quest (PC Engine) has six movement-panel commands: turn left/right,
move forward/backward and step left/right. Firestaff's Theron route accepts
both SDL scancodes and keycodes, so W/A/S/D also work on macOS layouts or
input paths that do not populate scancodes.

Theron gameplay runs in Firestaff's native C/SDL runtime. External emulators
are research instruments only and do not run or replace the game in Firestaff.
Native direct launch enters the Theron title/startup path without showing
Firestaff's Run menu or booting a System Card. A US Debug launch probe stopped
before dungeon loading (`levelLoaded=0`); loaded-dungeon evidence comes from
the separate JP fresh-start and US Continue routes. See the
[direct-launch note](../source-lock/theron-native-debug-direct-launch-20261031.md).
The original System Card's cold RUN callback has been traced separately. A
cold Japanese emulator route visibly reaches the Akutuba dungeon after a
four-second RUN hold at the System Card prompt. A separate isolated cold run
confirmed that a four-second Button I hold on NEW GAME opens file selection,
and another four-second hold on FILE 1 enters the Akutuba story; twelve
one-second presses then reach the dungeon. This verifies that JP title-action
hold in the original game. It does not establish native timing or a joined
game-owned input/CD-loader receipt. See the
[source-locked route](../source-lock/theron-jp-four-second-title-action-capture-20261031.md)
and [capture readiness](THERON_CAPTURE_READINESS.md).

## Reference PCE pad bindings

This table describes the Mednafen reference profile used for captures. It is
not Firestaff's native keyboard layout; native in-dungeon controls are listed
below.

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

The PC Engine retail manual documents the D-pad moving an on-screen cursor,
Button I acting on items and controls, and Button II cancelling. It describes
controller cursor movement but does not describe mouse input; see the
[TurboGrafx-16 instruction manual, pp. 4–6](https://www.tg-16.com/contributors/bernie/Dungeon_Master/Dungeon_Master.pdf).
A cold authentic Japanese Rev. 1 reference run then switched Mednafen port 1
to its PCE mouse device. A pair of dungeon screenshots differs only around the
original hand cursor after host-mouse movement, so visual cursor motion is now
confirmed. The PCE Fast module's mouse port is documented by
[Mednafen](https://mednafen.github.io/documentation/pce_fast.html). This
reference result does not identify absolute cursor coordinates, establish
click-to-command behavior, or prove native Firestaff parity.

Firestaff adds desktop mouse controls as a host-side convenience. During
startup, left-click selects and right-click goes back. During dungeon play,
left-click activates one of the twelve visible control-panel actions. Holding
right mouse while pressing a direction key applies the manual's Button II
simplified-movement modifier; a right click by itself does not advance a tick.
Middle-click is inert. Moving the pointer highlights a panel control. Clicks
outside the panel currently do not trigger a game action. Mapping desktop
coordinates to the original in-game cursor and its action consumer remains
open; the renderer must also update from native game state for world clicks to
show their result. Short and long touch gestures send Button I and Button II,
respectively.

## Startup menu

These are Firestaff's native-runtime bindings, not a PC Engine timing model.
On 2026-10-24, separate SDL key-down events advanced authentic JP media from
title through stage select and Soul Room into a loaded dungeon. The original
Japanese title-menu action was subsequently confirmed to advance after a
four-second Button I hold. That does not prove Firestaff requires or
reproduces the same hold duration; its native route remains driven by separate
SDL input events. The retail manual documents Button I as menu confirmation
but gives no multi-second timing rule. See
[DMWeb and Greatstone references](../DMWEB_REFERENCE.md), the
[capture report](../source-lock/theron-jp-four-second-title-action-capture-20261031.md),
and [TODO-theron.md](../../TODO-theron.md).

| Action              | Keys                        |
|---------------------|-----------------------------|
| Advance from title  | Z, Return, Space            |
| Navigate menu       | Up/Down Arrow, W/S          |
| Select stage        | Z, Return, Space            |
| Select/deselect hero| Z, Return, Space            |
| Enter forcefield    | Z, Return, Space            |
| Go back             | X, Escape                   |

## In-dungeon

Turn input reports `BLOCKED` and leaves facing unchanged if the bounded
source-actuator event queue cannot reserve the events required by the turn.
The runtime receipt does not request a redraw in that case. This is a
fail-closed queue-capacity guarantee, not evidence about the original game's
actuator timing or retail queue capacity; see the 2026-10-24 entry in
[`DONE-theron.md`](../../DONE-theron.md).

| Action         | Keys                              |
|----------------|-----------------------------------|
| Move forward   | Up Arrow, W, keypad 5/8           |
| Move backward  | Down Arrow, S, keypad 2           |
| Step left      | Left Arrow, A, keypad 1           |
| Step right     | Right Arrow, D, keypad 3          |
| Turn left      | Q, Home, keypad 4                 |
| Turn right     | E, End, keypad 6                  |
| Wait (tick)    | Z, Space, Return                  |
| Back/cancel    | X, Escape (ignored when no dungeon cancel surface is active) |
| Next champion  | Tab                               |
| Pick up facing item | G                            |
| Select next source item | I                         |
| Drop last picked source item | P                    |
| Send USE command (effect unverified) | U |

The native dungeon view also shows a twelve-button Firestaff mouse panel in
its lower-right corner. Its first two rows provide **TURN L**, **FORWARD**,
**TURN R**, **STEP L**, **BACK**, and **STEP R**. The remaining buttons are
**NEXT** champion, **PICKUP**, select the next source-backed **ITEM**, **DROP**
the selected source item, send the **USE** command for the facing object, and
**WAIT** one tick. The effect of USE on authentic doors and other objects is
not yet verified because the original T900 consumer remains unbound.
These buttons send the same native Theron input tokens as the corresponding
keyboard actions. Their captions use the active Theron translation catalog.
The panel is hit-tested in the native 320x200 game framebuffer after SDL
pointer coordinates have been mapped through the selected presentation mode.
At fractional scales, panel clicks are resolved against the actual presented
pixel bounds before the inverse transform so visible edge pixels remain
clickable.
Japanese and Chinese
captions use ASCII romanizations because the native small font does not
contain CJK glyphs.
Hovering a control highlights it. Pointer movement requests a redraw only when
the highlighted control changes.

## Notes

- During loaded gameplay, arrows and A/D issue the source-backed side-step
  commands `$06/$04`; Q/E, Home/End and keypad 4/6 turn. Keypad 5 and keypad
  8 move forward, with held keypad 8 sampled continuously instead of relying
  on SDL key-repeat events. Startup menu arrows retain their navigation
  behavior. This matches Firestaff's ordinary DM1
  dungeon keyboard layout while preserving Theron's native command tokens.
- SDL keyboard input routes the same supported loaded-dungeon commands as the
  native mouse panel: Tab cycles champion, G picks up, I selects the next
  source-backed item, P drops it, U sends the USE command for the facing
  object, and Space/Return or the PC Engine Button I binding Z waits. These
  keys enter the Theron input facade directly; they do not rely on the
  boot-probe-only key mapper.
- The native mouse panel provides all six Theron movement actions and the
  implemented champion, item, USE-command and wait inputs. The retail manual's
  Button II chord is mapped as right-mouse plus Left/Right Arrow, A/D, or
  keypad 1/3; the keypad 1/3 native SDL regression currently fails on
  hash-verified US Track 02 (Button II + keypad 1 produced `(1, 0, 3)` instead
  of `(1, 1, 3)`). Do not treat the keypad chord as verified until the runtime
  failure is fixed. A standalone right click does not advance a dungeon tick.
  In a loaded dungeon it is consumed only as the modifier, while Escape remains
  the desktop Back command. Existing arrow-chord and mouse-panel routes use
  authentic US runtime evidence. A native SDL mouse sequence through the visible
  host panel also turns twice and advances across a source-backed tile, using
  authenticated USA Track 02. Existing US and Japanese
  Rev. 1 Track 02 runtime tests also exercise the panel's turn and movement
  commands against each region's loaded dungeon state. The Japanese test
  asserts both click receipts and the resulting party pose. These checks cover
  Firestaff's host panel, not direct control of the original PCE cursor or
  world-space click behavior. The normal SDL-loop regression also uses the
  panel to pick up the authentic level-0 SANDALS, reselect that
  source-backed item and drop it;
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
  [`theron-original-turn-command-capture-2026-08-21.md`](../source-lock/theron-original-turn-command-capture-2026-08-21.md)
  and
  [`theron-original-forward-command-capture-2026-08-21.md`](../source-lock/theron-original-forward-command-capture-2026-08-21.md).
  Do not substitute DM1 hit regions or guessed coordinates.
- DM1-like mouse parity remains open: Theron's native dungeon view does not
  yet draw its original cursor, and clicks outside the Firestaff panel do not
  target the game world. The `$D515/$D588` interval candidate must be joined
  through the live pointer descriptor, hover result and Button-I command
  consumer before source-derived hit testing can be enabled. The viewport
  must also update from native game state for clicks to provide visible,
  end-to-end gameplay. See the current evidence and remaining capture gate in
  [`TODO-theron.md`](../../TODO-theron.md).
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
