# DM1 V1 Chest Occupied-Slot Leader/Non-Leader Swap Source Lock

Contract-only runtime regression gate for the open-chest C540 occupied-slot path where the leader hand starts with one weapon, C540 starts with another weapon, and a queued non-leader hand receive drains before the queued C540 pickup. The gate does not claim real-asset or original-DOS pixel parity.

## Source Anchors

- `CHEST.C F0333:30-75` opens the chest, keeps `G0426_T_OpenChest`, and materializes C537..C544 through `G0425_aT_ChestSlots`.
- `CHEST.C F0334:117-132` is a negative anchor for this pass: no close/relink path is allowed while the two queued slot commands drain.
- `CHAMPION.C F0297:243-268`, `F0298:270-298`, `F0300:511-515`, `F0301:606-660`, and `F0302:662-713` define the leader-hand remove/put, C30+ chest-slot clear/write, and slot-box swap route.
- `DUNGEON.C F0140:1114-1120`, `F0159:1664-1681`, `F0163:1769-1838`, and `F0164:1840-1905` anchor object weights, list next reads, append, and detach/rewire semantics.
- `OBJECT.C F0032:121-145` and `F0033:147-212` anchor object identity and icon resolution for the two weapon things.
- `COMMAND.C F0359:1452-1662` and `F0380:2045-2178` anchor queued mouse command write and drain into `F0302`.
- `IO.C F0077:1113-1122` and `F0078:1102-1111` bracket the two swap operations.
- `DEFS.H` anchors C30, C537..C544, `G0425`, `G0426`, `G0305`, `G0423`, `M070`, `M516`, `C0xFFFF`, and `C0xFFFE`.

## Pinned Contract

- Initial state: `G0426` names the open chest, C540 contains the chest weapon, the leader hand contains the leader weapon, and the non-leader ready hand is empty.
- The mouse queue writes C540 and non-leader-ready slot commands, then drains the non-leader receive before C540.
- The first drain moves the leader weapon into the non-leader hand through the `F0302`/`F0298`/`F0301` path.
- The second drain clears C540, moves the old C540 weapon into the leader hand through `F0300`/`F0297`, and rewires the still-open chain around the detached C540 entry.
- `F0334` is not called; the chest remains open and C537, C538, C539, C541, C542, C543, and C544 stay byte-stable.

## Disjointness

This is not the pass706 same-leader C538 occupied-slot swap, not the F0334 close rewire/stack-merge family, not a C071 eye close-then-open route, not a scroll-wheel route, not a C040/mirror-candidate/resurrect route, and not a party-rotation chest route.
