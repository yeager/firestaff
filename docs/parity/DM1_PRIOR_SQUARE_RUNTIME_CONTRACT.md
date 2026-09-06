# DM1 prior-square runtime contract

Source audit and runtime integration 2026-09-06. Runtime history is implemented
and covered by bounded regression tests; full original-execution parity remains
open.

## Authoritative transitions

- GROUP.C F0183:436-438 initializes PriorMapX/Y and HomeMapX/Y to the
  group's admitted location and LastMoveTime to game time minus 127.
- F0209:2164-2171 compares the candidate against prior coordinates and
  consumes a two-bit return-admission draw only on equality.
- F0209:2174-2181 commits prior coordinates to the event's source square
  only after the T0209061 move succeeds. A nonzero F0267 result returns
  before these writes. It also updates LastMoveTime.
- A delayed move changes movement timing but does not commit history
  (:2182-2185). Do not update prior coordinates merely on a planned move,
  a blocked candidate, or every generic F0267 call.

## Implemented ownership

M10 reaction/context builders and M11's wander/reaction adapters read the
nonserialized GameWorld active-group history sidecar. Each valid row is bound
to its C04 group index. Elapsed movement time uses the source byte wrap.
Successful physical F0209 movement commits its source coordinates; blocked
ordinary moves retain source coordinates in both active state and retry events.
Movement uses the selected move direction rather than the group's facing.

Admission initializes history, source-row publication validates coordinates,
retirement compacts history with its owner and clears the tail, and party-map
transition staging publishes history only with the staged state. Re-admission
replaces old history. The size-pinned serialized CreatureAIState is unchanged.
The direction/home source count is storage capacity, not the live group count.

Contexts without admitted history still use the legacy current-coordinate and
elapsed-time fallback. This fallback is not authenticated prior-square parity.

## Verified scope

- The M10 C29/C37 corridor regression executes two successful physical moves
  and checks square chains and history writeback. Events are explicitly
  scheduled by the test; this is not a natural original-emulator trace.
- Group-state tests cover source-row publication, byte-wrapped time, invalid
  coordinates and same-C04 re-admission.
- Explosion retirement tests cover history compaction and cleared tail rows.
- Ordinary-move tests cover terrain and occupancy rejection in all four
  directions, with unchanged source coordinates and no unlink/link operation.

## Required verification

Obtain original-execution comparisons for full M11 movement sequences and exact
RNG at subsequent decisions. Extend delayed-move, staged-failure and deferred
re-admission coverage. Audit all admission routes so missing history cannot
silently stand in for source state. Keep save restore provenance separate while
savegames are deferred. Bounded runtime tests do not prove all-platform parity.
