# DM1 V1 Chest Open + Mirror Candidate + Rotation Three-Way

Runtime-regression marker: `pass779_dm1_v1_chest_open_mirror_rotation_three_way`.

This is an asset-free, contract-only PC 3.4 compatibility gate. It does not
claim original-DOS pixel parity.

ReDMCSB anchors:

- `CHEST.C F0333:30-67` opens `G0426_T_OpenChest` and materializes
  `C537..C544` through `G0425_aT_ChestSlots`.
- `CHEST.C F0334:113-132` is the negative close/relink anchor for this gate.
- `CHAMPION.C F0297:243-298` and `F0298:270-298` own hand put/remove.
- `CHAMPION.C F0300:511-515` clears `C30+` chest slots.
- `CHAMPION.C F0301:606-614` writes `C30+` chest slots.
- `CHAMPION.C F0302:662-714` dispatches occupied slot-box clicks.
- `COMMAND.C F0359:1452-1662` queues mouse commands.
- `COMMAND.C F0361:1709-1813` writes keyboard/wheel-like queue commands.
- `COMMAND.C F0380:2045-2178` drains queued commands one at a time.
- `IO.C F0077:1113-1122` / `F0078:1102-1111` are the local tree locations
  for the mouse update bracket. A standalone `MOUSE.C` was not present in the
  local ReDMCSB tree.
- `REVIVE.C F0280:124-132` publishes the `C040`/`G0299` candidate.
- `REVIVE.C F0282:744-806` is the negative candidate-clear anchor here.
- `DUNGEON.C F0163:1796-1837` is the close-path relink anchor fed by `G0425`.
- `DEFS.H` binds `C30`, `G0425`, `G0426`, `G0423`, `G0305`, `M070`, `M516`,
  `C040`, `C160..C162`, `C537..C544`, and `C159`.

Scenario proved:

- Champion 1 owns a live open `G0426` chest and visible `C540` item.
- Champion 2 owns a live `C040` mirror candidate with a pending hand-queue item.
- A `C540` wheel command drains before a queued leader rotation.
- The wheel swaps the `C540` item into champion 1's hand and writes the old hand
  item back to `C540`.
- The `C040` candidate ordinal, candidate chain, and pending hand queue remain
  byte-stable across the wheel command and the later rotation.
- `G0426` and the chest panel remain open after the wheel and after rotation.
