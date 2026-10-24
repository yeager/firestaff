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

## Native Firestaff behavior

Firestaff maps keyboard direction inputs directly to the same four movement
commands, so it does not need a second modifier mode. The Button II host
bindings (right mouse, long touch, and cancel/back) must not advance the
Theron world tick by themselves. `Return` and `Space` remain separate host
wait/tick shortcuts. This source lock does not authorize item, combat,
transition, or rendering semantics.

Greatstone SCK remains a secondary format/extraction reference. It is not used
to infer this control contract, and its converted data does not replace
hash-verified regional Track 02 media.
