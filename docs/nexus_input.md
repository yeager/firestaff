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

## Firestaff keyboard routing

The M11 host maps W/S to forward/backward and Q/E to left/right turns. For
Nexus only, A/D are aliases for left/right turn, matching the observed retail
ControlPad behavior. This preserves DM1's A/D strafe mapping. Left/right arrow
keys and shoulder bindings remain the strafe inputs.

The Nexus movement-only `M11_GameView_HandleInput` route accepts movement
commands. The Nexus mechanics action gate remains fail-closed for unproven
retail actions. Do not treat the shared DM1 action, inventory, spell, pause, or
mouse routes as Nexus parity.

## Mouse and remaining input work

Nexus has startup pointer handling, but Firestaff does not yet have an
authenticated Nexus gameplay route from desktop pointer coordinates to retail
cursor actions or world commands. The existing logical click-route helpers
require pre-resolved targets and do not establish how viewport pixels map to
Nexus actions.

Next work must bind the Saturn cursor mode and non-movement buttons to
same-session retail input and state traces, then implement desktop mouse
control against those observed semantics. Keep unsupported actions inert until
their retail consumers are identified.
