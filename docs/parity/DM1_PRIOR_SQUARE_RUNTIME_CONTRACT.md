# DM1 prior-square runtime contract

Source audit 2026-09-06. Implementation remains open; helper tests alone
do not establish live anti-backtracking behavior.

## Authoritative transitions

- GROUP.C F0180:436-437 initializes PriorMapX/Y and HomeMapX/Y to the
  group's admitted location.
- F0209:2164-2171 compares the candidate against prior coordinates and
  consumes a two-bit return-admission draw only on equality.
- F0209:2174-2181 commits prior coordinates to the event's source square
  only after the T0209061 move succeeds. A nonzero F0267 result returns
  before these writes. It also updates LastMoveTime.
- A delayed move changes movement timing but does not commit history
  (:2182-2185). Do not update prior coordinates merely on a planned move,
  a blocked candidate, or every generic F0267 call.

## Current integration gap

M10 reaction/context builders substitute current group coordinates for
prior coordinates. M11's wander adapter explicitly does the same because
the serialized CreatureAIState record lacks history. This suppresses the
source return-admission draw. Production calls to the tested dispatcher
exist, but do not supply the missing state.

GameWorld already has nonserialized active-group direction and home-coordinate
sidecars. A history implementation must account for source-array staging,
active-row compaction/removal, map changes and C04 reuse. Do not change the
size-pinned serialized AI record merely to add runtime history.

## Required verification

Use two consecutive successful runtime moves, not only independently prepared
behavior contexts. Verify source and destination square chains, prior/history
writeback and exact RNG at the next decision. Cover failed/delayed moves,
active-row compaction and slot reuse so one group cannot inherit another's
history. Keep save restore provenance separate while savegames are deferred.
