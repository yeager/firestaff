# Nexus V1 Input Status

## Retail evidence

The current input observations use the Japanese retail Saturn disc and BIOS in
Ymir 0.4.1 with `--debug`. The evidence and paired screenshots are in
[`parity-evidence/verification/keyboard-mouse-debug-20261024/`](../parity-evidence/verification/keyboard-mouse-debug-20261024/README.md).

A second controlled run loaded a hash-verified authentic Japanese retail
gameplay savestate in a private Ymir profile. The no-input reload changed only
0.00175% of the captured pixels, while W-forward changed 3.08408% and A-left
changed 5.08610%. The before/after captures and source hashes are recorded in
[`parity-evidence/verification/nexus-savestate-input-20261025/`](../parity-evidence/verification/nexus-savestate-input-20261025/README.md).
This independently confirms retail ControlPad movement from a stable gameplay
state; it does not establish Firestaff runtime parity or ShuttleMouse actions.

The isolated Ymir ControlPad profile mapped `W/S/A/D` to Saturn D-pad
up/down/left/right, `J` to A, `O` to Z, and `F` to Start. In a fresh profile
with an empty BUP, F opened the title menu and S selected `CONTINUE`; a Right
shoulder event (keyboard R in that profile) did not change the menu selection.
Pressing J left the selected Continue row unchanged; no save-list transition
was observed in this empty-BUP profile. In the retail dungeon view, W moved forward, A turned left, D turned right, and F displayed
the game's `PAUSE` overlay. O entered the game's hand-cursor mode; W moved the
cursor upward in that mode. The captures show real game output from the retail
media, not generated or synthetic game state.

The ControlPad profile's desktop click did not produce a separately
attributable game action. A separate fresh profile configured Ymir port 1 as
`ShuttleMouse`: after entering mouse-capture mode, a mouse-button event was
followed by the retail title leaving its menu for the intro presentation. The
ShuttleMouse BUP remained blank. This limited title-flow result does not prove
pointer-coordinate targeting, in-dungeon mouse actions, or Firestaff mouse
behavior. Attack, inventory, spell, champion-selection, and other
non-movement button semantics remain unverified by these captures.

A later fresh-profile capture retained the actual orange Nexus hand cursor and
measured its movement after `O` (Saturn Z) and D-pad input. `W/S/A/D` moved it
up/down/left/right, and `W+A` moved it diagonally. The approximate per-press
displacements and authentic screenshots are recorded in
[`nexus-v1-hand-pointer-debug-20261025`](../parity-evidence/verification/nexus-v1-hand-pointer-debug-20261025/README.md).
The capture used Ymir fast-forward, so its host-time movement interval does
not establish a normal-speed repeat rate. It does not establish mouse
coordinates or button-to-action mapping.

## Retail manual control model

The Japanese retail manual's controller table (printed page 4) documents the
input model independently of the Ymir experiments: D-pad directions move or
turn in 3D mode and move the hand pointer in eight directions; L/R change the
leader; Start pauses; A attacks in movement mode and inspects, searches, uses,
or picks up in hand-pointer mode; B opens inventory; Y opens the magic window;
Z toggles the hand pointer; and C stores a held item in the backpack. In
hand-pointer mode, Y also moves the pointer to an item or switch. The manual's
inventory instructions (printed page 19) describe moving items between hand,
champion slots, and backpack with the hand pointer. See the
[Japanese retail manual, control table](https://api.regvault.org/api/v1/game/saturn/d83623212c7fd61623377cc9074bf3ea/manual#page=4)
and [inventory instructions](https://api.regvault.org/api/v1/game/saturn/d83623212c7fd61623377cc9074bf3ea/manual#page=19).

These documented controls define the retail interaction contract. Firestaff
implements the Z mode toggle and logical pointer movement in the captured
viewport. Inventory/magic panels and pointer targets remain unimplemented;
they must use Nexus-owned data and proven hit regions rather than DM1 panels
or guessed coordinates.

## Firestaff keyboard routing

The Nexus host keeps W/S for forward/backward and uses A/D to turn left/right,
matching the authenticated Ymir ControlPad capture. Left/right arrow keys
remain available for strafing through the shared M11 action map; Q/E and
Home/End also turn. DM1 retains its own A/D strafe bindings. The Nexus-specific
A/D mapping is evidence-based: Ymir mapped those keys to the Saturn D-pad, and
the retail D-pad turns in movement mode.

The retail game tick accepts only the six basic movement commands
(forward/backward, turn left/right, and strafe left/right) against the
authenticated DGN grid. In the loaded dungeon, Z toggles a host hand-pointer
mode. Arrow keys move its logical position in the authenticated DM.BIN
viewport rectangle; mouse motion positions it directly in that rectangle.
The hand sprite is sourced from the hash-verified retail ITEM.IBS image 65 and
palette 6, joined against Ymir `--debug` VDP1 texture capture. Mouse movement
and the six keyboard movement commands remain available with pointer mode off.
The authentic `DM.BIN` movement-pad rectangle is also wired as a six-cell
desktop movement surface: the upper row maps to turn left, forward, and turn
right; the lower row maps to strafe left, backward, and strafe right. This
host mapping uses the retail rectangle but does not claim to reproduce the
unresolved callback's cell semantics. Clicks elsewhere and hand-pointer clicks
remain inert. A/B/Y/C/L/R actions remain inert until their retail consumers
are traced. Item use, inventory, spells, combat, sensors, doors, and level
transitions remain fail-closed.

In a loaded Nexus dungeon, Space toggles Firestaff's host pause. Ymir evidence
records that the retail Start button displayed the `PAUSE` overlay; whether
retail gameplay resumes on a second press remains unverified. Firestaff's host
pause freezes the session tick and host audio until Space is pressed again.
This does not reproduce the retail `PAUSE` graphic or menu.

With pointer mode off, left-click within the authentic Nexus movement-pad
rectangle parsed from `DM.BIN` to send the corresponding desktop movement
command: the upper row maps to turn left, forward, and turn right; the lower
row maps to strafe left, backward, and strafe right. This host mapping uses a
retail rectangle but does not claim to reproduce Saturn ShuttleMouse or the
retail callback's unresolved cell semantics. Other buttons and clicks outside
the pad do nothing. With pointer mode on, mouse motion moves the logical hand
pointer and mouse clicks remain inert while callback semantics are unresolved.

`scripts/analyze_nexus_mouse_event_crossrefs.py` inventories direct SH-2
`MOV.L @(disp,PC),Rn` references in the hash-verified retail `DM.BIN`. In the
hash-pinned Japanese retail image, none of the 60 event-name strings has a direct
literal-pointer reference. The image does reference the HUD hit-rectangle
subtable beginning at file offset `0x38040`, and `yam\\mouse.c` appears in a
separate source-path reference. The string scan alone cannot resolve indirect
tables or generated pointers. The callback trace below follows one path beyond
those string references, but it still does not bind individual hit regions to
game commands.

The analyzer now also verifies one retail registration and dispatch chain.
SH-2 instructions at `DM.BIN:0x16588` load a callback at runtime address
`0x060264a8`, a companion table at `0x0604850c`, and the hit rectangles at
`0x06048040` (`DM.BIN:0x38040`), then call the setup routine at
`0x06034ad0`. That routine stores the supplied callback pointer in a global
slot; the paired dispatcher checks its active flag and makes an indirect call
through that slot. A second verified path at `DM.BIN:0x164a8` reads fields at
object offsets 100 and 104. When the first field is 6, submode 0 queues numeric
event 13 and submode 2 queues numeric event 44 to `0x06024288`; submode 1 does
not queue either event. The retail event-name pointer table maps ID 13 to
`EV_EXIT_INV` and ID 44 to `EV_INV_EQUIP`. The event-44 dispatch target is
identified below, but the downstream state change is not verified. The
analyzer checks the branch targets, event values, queue target, and name
pointers against the hash-verified Japanese retail `DM.BIN`.

The same retail image contains an eight-entry action-rectangle table at file
offset `0x3850c`. Each 12-byte record stores `(x1, y1, x2, y2, event, slot)`;
the eight authentic records all use event 44 and slot arguments 0 through 7.
The SH-2 hit scanner at file offset `0x24ed6` compares pointer coordinates
against those bounds, reads the event and slot fields, and queues them to
`0x06024288`. This binds the rectangles to `EV_INV_EQUIP` with a slot argument.
The verified consumer at file offset `0x21c84` has special branches only for
event IDs 38, 39, 40, 49, 52, 56, and 57. IDs 13 and 44 branch at `0x21cce`
to the generic diagnostic at `0x21d98`, which resolves the event name and
emits `bad event data`. This rules out an inventory mutation in this queue
consumer; it does not rule out a separate consumer elsewhere in the retail
runtime.
The fallback calls `DM.BIN:0x141b4`, which bounds-checks the event ID and
indexes the authentic name-pointer table at runtime address `0x0604adc4`.
It returns a name string; it does not dispatch an event handler. The analyzer
now verifies this helper's instructions and table base in the pinned image.
The callback's event-44 producer, the event-17 producer behind a separate
global-slot gate, and the dispatcher share the same bounded queue: eight
12-byte records at runtime `0x060797b0`, with count/state at `0x06079814` and
dequeue helper `0x060242d6`. The later queue loop described above is not
necessarily the only consumer; event-action semantics must be followed from
each caller's event context.
An even-offset scan of the pinned image finds no raw `CMP/EQ #44,R0` opcode
candidate. The analyzer labels this as an opcode-pattern scan only: it cannot
distinguish data from code or exclude register comparisons, jump tables, or
indirect event dispatch.
An early main event dispatcher at file offset `0x260` uses a relative jump
table. Its event-44 case reaches file offset `0x8c0`, where a checked
PC-relative literal points to code at runtime address `0x06036e58`; the hit-row
argument is passed in `R5`. A separate check verifies selected instruction
words and address literals in nearby handler/helper regions, including values
`0x013c`, `0x0607fba0`, `0x0110`, `0x06064610`, and `0x1000`. These checks do
not verify control flow from this dispatch case through those regions, decode
their full operations, or establish any memory write or callback effect.
Accordingly, the slot-pair and assignment/swap interpretation remains a
hypothesis, not a verified retail behavior; neither its Firestaff field
mapping nor its visible result is known. This dispatch path is separate from
the later `0x21c84` generic diagnostic loop.
The separate mouse-motion poller filters for event 46 before it calls the
hit-region scanner; it does not consume event 44 as a second pointer action.
The event-44 dispatch target is verified, but its downstream behavior is not.
Production inventory and equipment clicks remain disabled until the retail
state transition, Firestaff field mapping, and visible result are verified.

## Mouse and remaining input work

Nexus has startup pointer handling, viewport mouse movement, keyboard
movement, and the authentic retail cursor sprite in the captured viewport.
The keyboard supports forward/backward movement, turning, strafing, pointer
mode, and pointer movement. The mouse supports the six-cell desktop movement
pad and direct pointer positioning. Inventory, attack, spell, and item-use
clicks remain inert until their retail callback consumers are established.
The existing logical click-route helpers require pre-resolved targets and do
not establish how viewport pixels map to Nexus actions. The orange retail
cursor and D-pad movement are captured, while desktop mouse movement in
Firestaff is a host control behavior rather than a claim about Saturn
ShuttleMouse.

Next work must map the retail context/slot pair to the authentic Nexus
champion/inventory model and verify the post-click visible result. Then bind
the documented cursor and non-movement controls to those state transitions in
the same session. Keep unsupported actions inert until that mapping is proven.
