# Pass 1111 — DM2 creature AI module (c_1c9a.cpp)

## Source

skproject `SKULLWIN/c_1c9a.cpp` — 10,022 lines, 49 functions.

## Module

- Header: `include/dm2_v1_1c9a_pc34_compat.h`
- Source: `src/dm2/dm2_v1_1c9a_pc34_compat.c`
- Test: `tests/test_dm2_v1_1c9a_pc34_compat.c`

## Scope

Creature AI module covering:

- **Tile passability** (DM2_1BAAD, DM2_1BA1B, DM2_1BC29): tile type checks
  for creature movement — open floor, doors (with rebirth altar check),
  teleporters (open bit), walls, record-list walking for creature/item blocking.
- **Tile cache** (DM2_19f0_045a, 04bf, 050f): cached tile value and record
  chain queries for repeated access to the same tile.
- **Line walk** (DM2_19f0_0207): Bresenham-style line-of-sight check with
  pluggable tile check callback.
- **Turn selection** (DM2_19f0_0559): creature facing direction decision
  (left/right turn toward target direction).
- **Popcount** (DM2_1c9a_0598): bit count utility for AI flag evaluation.
- **Direction normalization** (DM2_19f0_1511): sign-extend lower 16 bits.
- **Timer management** (DM2_1c9a_0cf7, 0db0): creature AI timer scheduling
  and cancellation via callback delegation.
- **CAII allocation** (DM2_ALLOC_CAII_TO_CREATURE): delegates to caii_alloc
  module.
- **Creature activation** (DM2_1c9a_0fcb): wakeup with CAII alloc + timer.
- **Distance** (DM2_1c9a_17c7): creature-to-party Manhattan distance.
- **Map change** (DM2_1c9a_0648): CAII lookup with map switch.

### Fail-closed stubs (24 functions)

The following functions are structurally bound with correct signatures,
callback patterns, and receipt structs, but their bodies are fail-closed
stubs pending full source porting:

- DM2_19f0_05e8 — combat/movement detailed evaluation (284 lines)
- DM2_19f0_0891 — attack/movement decision engine (700 lines)
- DM2_19f0_0d10 — movement decision with flee/approach (594 lines)
- DM2_19f0_13aa — combat target selection (168 lines)
- DM2_D283 — creature record lookup (72 lines)
- DM2_CREATURE_GO_THERE — movement orchestrator (1,458 lines)
- DM2_19f0_2024 — pre-movement tile validation (134 lines)
- DM2_19f0_2165 — AI tick handler (514 lines)
- DM2_19f0_266c — ranged attack feasibility (77 lines)
- DM2_19f0_2723 — adjacent tile threat assessment (117 lines)
- DM2_19f0_2813 — path obstruction check (240 lines)
- DM2_1c9a_0247 — CAII cleanup (25 lines)
- DM2_1c9a_06bd — creature record by position (27 lines)
- DM2_1c9a_078b — group movement (126 lines)
- DM2_1c9a_0958 — creature type extractor (24 lines)
- DM2_1c9a_09b9 — creature property getter (9 lines)
- DM2_1c9a_09db — creature facing update (15 lines)
- DM2_CREATURE_SOMETHING_1c9a_0a48 — global AI loop (259 lines)
- DM2_CREATE_MINION — spawn summoned creature (185 lines)
- DM2_RELEASE_MINION — destroy summoned creature (30 lines)
- DM2_1c9a_1a48 — creature damage (79 lines)
- DM2_1c9a_1b16 — creature healing (62 lines)
- DM2_FIND_WALK_PATH — A* pathfinding (3,229 lines)
- DM2_1c9a_381c — walk path consumer (49 lines)
- DM2_1c9a_38a8 — CAII table compaction (145 lines)
- DM2_1c9a_19d4 — creature position update (31 lines)
- DM2_FILL_CAII_CUR_MAP — map creature init (97 lines)
- DM2_FILL_ORPHAN_CAII — orphan CAII recovery (26 lines)

## Architecture

Callback-based: all functions take `const DM2_V1_1c9aCallbacks *cb, void *ctx`.
The callback struct provides 45 function pointers covering map queries,
creature queries, distance/direction, random, party, CAII, record allocation,
DB allocation, and ddat state access.

Receipt structs capture observable output for every function with non-trivial
return semantics.

Module-local tile cache (`DM2_V1_1c9aTileCache`) mirrors the source's
ddat.v1e08a8-v1e08c4 cached tile state.

## Test results

52 tests, 52 passed, 0 failed.

Covers: popcount, direction normalization, tile cache init, tile passability
(open floor, wall, teleporter open/closed, type 7, door sub-type 4),
party shortcut, tile cache refresh/hit/miss, record chain empty-list,
creature turn, timer scheduling/cancellation, CAII delegation, creature
activation, distance calculation, map change, all stub fail-closed returns,
direction tables, and null-callback safety for all 13 proven functions.

## Proven parity

| Function | Lines | Status |
|----------|-------|--------|
| DM2_1c9a_0598 (popcount) | 933-957 | Proven verbatim |
| DM2_19f0_1511 | 2430-2435 | Proven verbatim |
| DM2_1BAAD | 23-150 | Proven (tile type routing) |
| DM2_1BA1B | 5090-5133 | Proven (tile type dispatch) |
| DM2_1BC29 | 152-160 | Proven (party shortcut) |
| DM2_19f0_045a | 470-501 | Proven (cache logic) |
| DM2_19f0_04bf | 503-540 | Proven (chain walk) |
| DM2_19f0_050f | 542-573 | Proven (creature find) |
| DM2_19f0_0559 | 584-645 | Proven (turn selection) |
| DM2_1c9a_0cf7 | 5695-5732 | Proven (delegation) |
| DM2_1c9a_0db0 | 5734-5763 | Proven (delegation) |
| DM2_ALLOC_CAII | 5772-5894 | Proven (delegation) |
| DM2_1c9a_0fcb | 5896-5958 | Proven (alloc+schedule) |
| DM2_1c9a_17c7 | 6182-6239 | Proven (distance) |
| DM2_1c9a_0648 | 5162-5196 | Proven (map change) |
| 24 stubs | various | Fail-closed, structurally bound |
