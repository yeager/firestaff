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

A separate controlled gameplay-state sequence mapped `K` to ControlPad B and
`J` to A. The `--debug` high-WRAM dumps show B queueing event 12
(`EV_INVENTORY`). A in that flow queues event 39 (`EV_RING_SELECT`) with
argument 8; the visible retail view changes to an equipment page labelled
`頭に装備` (“equip to head”). Context record 10 changes at `+0x122/+0x124`,
but the capture does not join that record to a champion or prove a persistent
item transfer. The screenshots, complete WRAM dumps, source hashes and event
trace are recorded in
[`nexus-v1-inventory-button-debug-20261025`](../parity-evidence/verification/nexus-v1-inventory-button-debug-20261025/README.md).
Retail disassembly clarifies the boundary: event 12 validates and updates a
global UI selection list, while event 39 invokes a callback on the current UI
view to change pages. Neither handler writes a champion's equipment slots in
this capture, and the candidate champion slot pairs remain unchanged. Event 44
(`EV_INV_EQUIP`) is a separate retail slot-action route: its handler reads and
writes context slot pairs and checks item compatibility, but its live runtime
result and context-to-champion mapping have not yet been captured. The button
trace therefore establishes menu navigation only; Firestaff inventory and
equip actions remain disabled until event 44's runtime effects and the
champion/item joins are verified.

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

A Ymir `--debug` run on the authentic Japanese retail disc independently
confirmed L/R leader changes: the configured L input selected the fourth
party member and R returned to the first. The before/after game captures are
in [`nexus-leader-cycle-debug-20261025`](../parity-evidence/verification/nexus-leader-cycle-debug-20261025/README.md).
Firestaff routes its existing Tab “cycle champion” input through the loaded
Nexus party order, matching the manual's leader-change control. The captures
establish the retail behavior, but do not identify the guest RAM leader field.
The retail PLRD roster has no live/dead field, so Firestaff does not treat its
zero-initialized runtime `alive` member as proof that a selected champion is
dead. Leader cycling accepts a roster-backed party member with positive
current health, and skips members whose runtime health is zero. This keeps the
authentic start roster selectable without inventing liveness data.

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
mode. W/A/S/D and the arrow keys move its logical position across the
authentic `NEXUS_HIT_FULL_GAME_SCREEN` rectangle parsed from `DM.BIN`;
simultaneous directions allow diagonal movement. Firestaff clips its lower
edge to the 320×200 source framebuffer. This lets the visible hand cursor move
from the viewport over the Nexus HUD. The W/A/S/D mapping matches the captured
retail profile's D-pad bindings. Mouse motion positions the hand pointer
directly across the same screen area.
The hand sprite is sourced from the hash-verified retail ITEM.IBS image 65 and
palette 6, joined against Ymir `--debug` VDP1 texture capture. Mouse movement
and the six keyboard movement commands remain available with pointer mode off.
The authentic `DM.BIN` movement-pad rectangle is also wired as a six-cell
desktop movement surface: the upper row maps to turn left, forward, and turn
right; the lower row maps to strafe left, backward, and strafe right. This
host mapping uses the retail rectangle but does not claim to reproduce the
unresolved callback's cell semantics. Clicks elsewhere and hand-pointer clicks
remain inert. A/B/Y/C actions remain inert until their retail consumers are
traced. Tab provides the separately verified leader-cycle action; L/R
controller events are not yet mapped to desktop controls. Item use, inventory,
spells, combat, sensors, doors, and level transitions remain fail-closed.

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
pointer across the viewport and HUD. Mouse clicks remain inert while the
callback's HUD-region action semantics are unresolved.

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
`EV_EXIT_INV` and ID 44 to `EV_INV_EQUIP`. The event-44 dispatch target and
slot-pair access/write path are verified below, while its held-item UI effects
and Firestaff state mapping remain unverified. The
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
12-byte records at runtime `0x060797b0`, count at `0x06079814`, and queue state
at `0x06079810`; the dequeue helper is `0x060242d6`. The later queue loop is not
necessarily the only consumer; event-action semantics must be followed from
each caller's event context.
An even-offset scan of the pinned image finds no raw `CMP/EQ #44,R0` opcode
candidate. The analyzer labels this as an opcode-pattern scan only: it cannot
distinguish data from code or exclude register comparisons, jump tables, or
indirect event dispatch.
An early main event dispatcher at file offset `0x260` uses a relative jump
table. The event-44 case reaches `0x8c0`, loads the slot argument from stack
offset 56, and calls the verified handler at runtime address `0x06036e58`.
That handler passes the context index through a resolver that uses base
`0x0607fba0` and stride `0x13c`. For ordinary slots 0–6, the helper accesses
two 16-bit words at `context + 0x110 + 4*slot`, stages and clears the source
pair, checks item metadata through the 40-byte records at `0x06064610`, and
writes the staged pair to a destination slot on the compatible path. Slot 7
uses a separate path. The handler also updates context fields using masks
`0x1000` and `0x0200`. The checker verifies the key SH-2 instructions,
branches, helper calls, table literals and pair reads/writes against the
hash-pinned retail `DM.BIN`.

The first pair word is an item-definition ordinal; `0xffff` is the empty
marker. There is a partial fit with Firestaff's ITEM.IBS declaration indices,
but not a lossless mapping. The context index is not joined to a champion,
Firestaff's champion model has no field for the opaque second 16-bit word, and
the retail PLRD item pairs do not establish the inventory-panel layout. The
hash-pinned Japanese `RLOWFIX.BIN`
(`f2686bf3d6b971c5eaa613b2619b7e1bb7958a8045876296a83509f137be18b0`) contains
20 PLRD rows with ten item pairs each: 64 pairs have an item ordinal, 136 are
empty, and all 200 second words are zero. A read-only comparison of four
authentic Ymir captures found that context records 0–3 duplicate each other
and match PLRD row 2's first seven pairs, but differ at slot 7; empty context 4
matches four PLRD rows. These ambiguous matches do not establish a unique
context-to-champion mapping or the auxiliary word's meaning for live items.
The candidate relation and capture hashes are recorded in
[`nexus-v1-inventory-button-debug-20261025`](../parity-evidence/verification/nexus-v1-inventory-button-debug-20261025/README.md).
The helper effects that transfer the held item through global UI state and
the runtime-visible result still need a controlled Ymir before/after capture.
The analyzer reproduces the DM.BIN instruction checks and PLRD pair summary.
This path is separate from the later `0x21c84` generic diagnostic loop.
The separate mouse-motion poller filters for event 46 before it calls the
hit-region scanner; it does not consume event 44 as a second pointer action.
Event 44 has a hash-verified source-side slot-pair path, but its runtime
held-item effects and Firestaff state mapping are not verified. Production
inventory and equipment clicks remain disabled until those joins are captured.

## Mouse and remaining input work

Nexus has startup pointer handling, full-game-screen mouse movement, keyboard
movement, and the authentic retail cursor sprite joined from the captured
viewport data and ITEM.IBS source.
The keyboard supports forward/backward movement, turning, strafing, party
leader cycling (Tab), pointer mode, and pointer movement. The mouse supports
the six-cell desktop movement pad and direct pointer positioning across the
game screen. Inventory, attack, spell, and item-use
clicks remain inert until their retail callback consumers are established.
The existing logical click-route helpers require pre-resolved targets and do
not establish how viewport pixels map to Nexus actions. The orange retail
cursor and D-pad movement are captured, while desktop mouse movement in
Firestaff is a host control behavior rather than a claim about Saturn
ShuttleMouse.

Next work must identify the retail context-to-champion relationship and the
opaque item-pair auxiliary word, then trace whether the event-39 selection
commits a retail inventory mutation and how the relevant item maps to the
authentic Nexus champion/inventory model. Bind pointer actions only after
those joins are proven. Keep unsupported actions inert. The attempted
ShuttleMouse click and its unmatched WRAM snapshots are preserved in
[`parity-evidence/verification/nexus-keyboard-mouse-retail-input-20261025/`](../parity-evidence/verification/nexus-keyboard-mouse-retail-input-20261025/README.md);
they do not prove an action.

A later isolated Ymir probe used the authentic slot-1 state with ControlPad on
port 1 and ShuttleMouse on port 2. XTEST core-pointer motion and the ShuttleMouse
port's relative arrow-key bindings produced no visible movement of a clearly
identifiable retail hand pointer in the Xvfb captures. The setup and captures
are recorded in [`nexus-shuttle-mouse-xvfb-20261025`](../parity-evidence/verification/nexus-shuttle-mouse-xvfb-20261025/README.md).
The XTEST result is explained by a Ymir 0.4.1 frontend gap: SDL global mouse ID
`0` enters relative-capture mode and returns before Ymir maps its motion or
buttons to the emulated ShuttleMouse. Ymir PR
[#963](https://github.com/ymir-emu/Ymir/pull/963) proposes connecting that
stream when a ShuttleMouse is selected; it still needs a full build and runtime
`--debug` verification. Neither the earlier capture nor the proposed fix
establishes Nexus retail mouse semantics. Firestaff's desktop mouse actions
still need to follow proven Nexus state changes before enabling equipment or
other pointer actions.
