# DM1 V1 Chest Scroll-Wheel Close Race Runtime Gate

This pass768 runtime regression covers one narrow race: champion 0 has an open
DM1 V1 chest, a C540 scroll-wheel slot command and a leader-rotation command are
queued through the command path, then the same champion closes the chest before
`F0380_COMMAND_ProcessQueue_CPSC` drains the queued slot command. The stale slot
command must be rejected against closed `G0426`, the closed `C537..C544` chain
must be preserved, the leader hand must survive the later rotation coherently,
and the `F0077`/`F0078` mouse update bracket must balance.

Source anchors:

- `CHEST.C F0333:30-67`: opens `G0426_T_OpenChest` and materializes visible
  linked chest objects into `G0425_aT_ChestSlots` / `C537..C544`.
- `CHEST.C F0334:113-132`: clears `G0426_T_OpenChest` and relinks non-empty
  visible `G0425` entries back into the container list.
- `CHAMPION.C F0297:243-268` and `F0298:270-298`: own the global `C030` leader
  hand across the stale command and queued rotation.
- `CHAMPION.C F0301:606-614` and `F0302:662-714`: route C30+ chest slots through
  `G0425` and dispatch slot-box commands.
- `COMMAND.C F0359:1452-1662`: queues mouse commands without draining them.
- `COMMAND.C F0380:2045-2178`: drains queued slot-box commands and leader
  command dispatch.
- `IO.C F0077:1113-1122` and `F0078:1102-1111`: bracket mouse/screen update
  suppression.
- `DEFS.H:267,790,2088,810,1876,3906-3913,5878,5881`: `C030`,
  `C10_SLOT_NECK`, `C10_COLOR_FLESH`, `C30`, `C38`, `C537..C544`, `G0425`,
  `G0426`, including the selected `C540` route.

Non-duplicative claim:

This is not the pass767 chest-deposit-during-leader-rotation or mirror-candidate
cancel-with-rotation slice. It also avoids the existing reopen, pickup/drop,
capacity, encumbrance, save/load, teleporter, mirror-candidate,
resurrect-pending, empty-party, empty-reopen, hidden-tail, occupied-hand,
open-while-another, partial-mask, link-corruption, non-leader-hand,
cross-champion, max-weight, occupied-slot swap, and generic scroll-wheel pickup
or pull-from-chest cases. The only asserted race is stale C540 scroll-wheel
queue drain after same-champion close and before queued leader rotation
completion.
