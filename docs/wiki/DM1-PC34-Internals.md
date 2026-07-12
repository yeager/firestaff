# DM1 PC34 Internals

## Compatibility Boundary

This page describes the V1 PC 3.4 compatibility route. It is intentionally
different from a generic dungeon engine: Firestaff preserves source ownership
and only moves a responsibility from M11 to M10/DM1 when the input and output
contract can be stated in source terms.

| Layer | Owns | Must not own |
|---|---|---|
| M12 | source discovery, selected game/profile, launch facts | dungeon mutation |
| M11 | SDL events, frame presentation, private host surfaces | Thing semantics |
| M10 | loaded maps, Thing chains, timeline, transactional saves | generated art |
| DM1 V1 | PC34 decision, render, and input receipts | unverified asset fallback |

## Source Data Contract

The required PC34 pair is identified by content hash. `GRAPHICS.DAT` is opened
only after the scanner has recorded a supported identity; `DUNGEON.DAT` is
loaded through the same verified profile. The runtime can receive an extracted
path from an archive cache, but archive member names are never substituted for
hash identity.

The indexed framebuffer remains source-sized at 320x200. A renderer request is
valid only when it names a source graphic, palette/indexed material, rectangle,
and destination layer. V2 scaling/filtering happens after this V1 decision
boundary and cannot repair a missing source draw.

## Dungeon Chains

PC34 map squares point to linked Things. The crucial invariant is that every
list edit goes through the loaded-chain primitive:

1. find the source link and predecessor in the source map;
2. unlink or remove it while preserving the source tail rule;
3. switch map context only for the bounded destination operation;
4. append to the destination tail, never silently replace its head;
5. write changed next-links to the decoded raw Thing representation;
6. restore the caller's current-map context.

This is the same ownership split used by F0267 routes. Pit and teleporter
events may enumerate an initial snapshot of a square's ordinary Things, but
must not traverse a chain while unlinking/relinking from that chain.

## Timeline and Sensor Dispatch

The timeline dispatcher accepts source event classes, not arbitrary host
callbacks. Square-state C07/C08/C09/C10 handling changes only the documented
fakewall, teleporter, pit, and door state fields. F0248 wall batches preserve
list order and represent C005/C006 writes, C007-C010/C014/C015 launcher
requests, and C018 endgame requests as typed outcomes.

F0276 sensor evaluation is separate from F0268 effect publication. The remote
effect must carry a resolved target map/cell and is queued as a timed square
state event. This prevents a sensor callback from mutating a map while an
unrelated source traversal still owns the chain.

## Group Sight and Action Scheduling

The live C37 path combines ReDMCSB `F0200`, `F0197`, `F0199`, and `F0227`.
The result is not a Euclidean distance check. The resolver evaluates:

* group and party direction cones;
* straight and fixed-point diagonal tile traversal;
* both corner blockers at equal-axis diagonal boundaries;
* walls, closed fakewalls, door openness and exceptional transparent types;
* current dungeon darkness and PC34 invisibility/SEE_INVISIBLE flags;
* bounded source near-party awareness decisions.

Only the resulting visibility receipt may drive group move/flee/attack
scheduling. A false result must not preemptively allocate a projectile or alter
the group chain.

## F0115 Render Plans

F0115 begins with static Things and then has separate live passes. M11 exposes
only the static square chain and typed M10 projectile/explosion arrays. DM1
filters active same-square instances, maps source aspect/frame choices, and
returns a draw plan. Hall of Champions mirror payloads are explicitly excluded
from the ordinary item/effect path.

This division is important: UI ownership does not imply dungeon ownership. A
host may draw a plan, but it cannot create a plan by scanning untyped memory.

## Save Import Transaction

PC34 import is candidate-first. Header, party, active groups, event queue,
timeline, and optional tail data are decoded into a detached candidate. Every
range and queue index must validate before the live world is replaced. The same
rule applies to backup promotion and original export/reload checks.

In particular, a malformed EVENT/TIMELINE record cannot leave a partly updated
live queue. This is a correctness invariant, not merely input hardening.

## Focused Verification

```bash
./build/test_dm1_v1_group_visible_distance_pc34_compat
./build/test_dm1_v1_square_state_dispatch_pc34_compat
./build/test_dm1_v1_projectile_explosion_render_pc34_compat
./build/test_dm1_v1_original_save_pc34_handoff
```

Use a real PC34 data pair for final presentation capture. Headless/unit proof
does not establish Mac window composition, timing, or asset availability.
