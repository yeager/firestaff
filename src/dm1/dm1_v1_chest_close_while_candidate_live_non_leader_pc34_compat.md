# DM1 V1 Chest Close While C040 Candidate Live Non-Leader Gate

This is an asset-free runtime contract for a narrow DM1 V1 regression:

- Champion 1 owns the open chest (`G0426`) and is not the leader.
- Champion 2 owns a live C040 resurrect/reincarnate mirror candidate.
- The close button closes only champion 1's visible `G0425` chest slots.
- The live C040 candidate ordinal, owner, slot, and panel chrome survive.
- A close-time C039 panel click is rejected before it can re-enter `F0333`.

Source anchors:

- `CHEST.C F0333:30-67` materializes the first eight visible chest links.
- `CHEST.C F0334:113-132` clears `G0426`, relinks non-empty visible slots,
  and truncates hidden tail entries.
- `CHAMPION.C F0297:243-298`, `F0298:270-298`, `F0300:511-515`,
  `F0301:606-614`, and `F0302:662-714` define leader-hand and `C30+`
  transfer routes that this close-only gate must not trigger.
- `REVIVE.C F0280:124-132` publishes the candidate; `F0282:744-806`
  clears it only through resurrect/reincarnate/cancel.
- `COMMAND.C F0359:1985-1990` owns the live C040 panel dispatch.
- `DEFS.H` binds `C040`, `C537..C544`, `C030`, `G0425`, and `G0426`.

The gate is deliberately disjoint from the pickup and party-rotation passes:
it performs no C537 pickup, no pending-hand queue resolution, no inventory owner
swap, and no party rotation. It only exercises the non-leader close path while
another champion's C040 candidate remains live.
