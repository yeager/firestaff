# Nexus V1 Input Status

## Retail evidence

The current input observations use the Japanese retail Saturn disc and BIOS in
Ymir 0.4.1 with `--debug`. The evidence and paired screenshots are in
[`parity-evidence/verification/keyboard-mouse-debug-20261024/`](../parity-evidence/verification/keyboard-mouse-debug-20261024/README.md).

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

These documented controls define the retail interaction contract. The current
Firestaff runtime does not yet implement the Nexus hand-pointer mode,
inventory/magic panels, or their pointer targets, and the Ymir captures above
do not establish those routes. They must be implemented with Nexus-owned data
and proven hit regions rather than by reusing DM1 panels or guessed
coordinates.

## Firestaff keyboard routing

The M11 host uses the same desktop keyboard layout for Nexus as for DM1:
W/S move forward/backward, A/D and left/right arrows strafe, and Q/E or
Home/End turn. This is a host keyboard layout; Saturn D-pad left/right turn
semantics remain specific to a connected Saturn ControlPad.

The retail game tick now accepts only the six basic movement commands
(forward/backward, turn left/right, and strafe left/right) against the
authenticated DGN grid. Item use, inventory, spells, combat, sensors, doors,
and level transitions remain fail-closed.

In a loaded Nexus dungeon, Space toggles Firestaff's host pause. Ymir evidence
records that the retail Start button displayed the `PAUSE` overlay; whether
retail gameplay resumes on a second press remains unverified. Firestaff's host
pause freezes the session tick and host audio until Space is pressed again.
This does not reproduce the retail `PAUSE` graphic or menu.

For desktop mouse movement, Firestaff uses the authentic Nexus viewport
rectangle parsed from `DM.BIN` as a host control surface. Left-click the upper
left/right thirds to turn, the lower left/right thirds to strafe, and the upper
or lower center third to move forward or backward. This is a Firestaff desktop
mapping into the same six keyboard movement commands; it does not claim to
reproduce Saturn ShuttleMouse or retail hit-table callback behavior. Clicks
outside that viewport and right-clicks do not enter DM1 gameplay routes.

`scripts/analyze_nexus_mouse_event_crossrefs.py` inventories direct SH-2
`MOV.L @(disp,PC),Rn` references in the hash-verified retail `DM.BIN`. In the
current English retail image, none of the 60 event-name strings has a direct
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
through that slot. This proves that the retail program registers and invokes
a callback associated with the authentic hit-region data. The callback's
state transitions and the action assigned to individual rectangles are still
unresolved, so this evidence is not enough to implement click actions.

## Mouse and remaining input work

Nexus has startup pointer handling and now has desktop mouse movement mapped to
the verified basic movement command set. Firestaff still does not have an
authenticated Nexus gameplay route from pointer coordinates to cursor actions,
inventory/magic panels, or world interactions. The existing logical
click-route helpers require pre-resolved targets and do not establish how
viewport pixels map to Nexus actions.

Next work must bind the documented cursor and non-movement controls to
same-session retail input and state traces. Keep unsupported actions inert
until their retail consumers are identified.
