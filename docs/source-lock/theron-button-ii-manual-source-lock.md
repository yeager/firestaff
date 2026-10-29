# Theron's Quest Button II control reference

## Primary reference

The USA retail instruction manual, printed page 5, defines Button II as
Cancel. Its “Simplified Movement” section says to hold Button II while pressing
the direction key: Up moves forward, Down moves backward, Right turns right,
and Left turns left. Page 4 describes Button I as the title/stage selection
control. The manual does not prescribe a multi-second hold for menu selection.

Source: [Theron's Quest USA instruction manual](https://www.tg-16.com/contributors/bernie/Dungeon_Master/Dungeon_Master.pdf),
also indexed from the [DMWeb Theron's Quest PC Engine edition page](http://dmweb.free.fr/games/therons-quest/editions/pc-engine-turbografx/).
The scanned manual is external reference material; no manual image or game
media is copied into the repository.

The manual documents the PCE direction key and controller buttons but does not
describe mouse input. A [DMWeb Encyclopaedia forum answer](https://www.dungeon-master.com/forum/viewtopic.php?t=23303)
states that mouse input is unavailable in the original TurboGrafx version.
That broad statement is superseded by the authentic 2026-10-29 PCE Fast
reference capture, which shows that PCE mouse motion moves the original hand
cursor. The capture does not bind guest coordinates or establish click-to-
command behavior; see the
[pointer-motion capture note](theron-disassembly/theron-jp-mouse-pointer-motion-20261029.md).

Firestaff's current desktop mouse panel remains a host-side convenience that
dispatches supported native commands. Its rectangles are Firestaff UI
geometry, not recovered game hitboxes. Do not infer viewport mouse targets
from DM1's regions or the PCE cursor's internal coordinates.

## Native Firestaff behavior

Firestaff maps keyboard direction inputs directly to the same four movement
commands. In a loaded dungeon, holding the right mouse button (host Button II)
while pressing Left/Right Arrow or A/D maps those directions to turns instead
of side-steps, matching the manual's simplified-movement chord. The modifier
clears on right-button release or focus loss. A standalone Button II press,
long touch or cancel/back input must not advance the Theron world tick.
`Return` and `Space` remain separate host wait/tick shortcuts. This source
lock does not authorize item, combat, transition, or rendering semantics.

Greatstone SCK remains a secondary format/extraction reference. It is not used
to infer this control contract, and its converted data does not replace
hash-verified regional Track 02 media.
