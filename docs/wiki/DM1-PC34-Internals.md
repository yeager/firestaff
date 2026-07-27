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

## Original Save Corpus Runtime Gate

An externally supplied PC34 save corpus is validated independently of the
normal resume path. Each qualified source is read from an immutable snapshot,
staged through the F0435 order, exported only to transient memory, and
reloaded. A source reaches the corpus runtime receipt only when its own saved
dungeon tail materializes an owned dungeon. That candidate is then adopted
into a blank runtime world; no start-world dungeon is supplied at either step.

Tail-less saves may still use the ordinary resume route with the verified
DM1 start dungeon, but they are reported as unavailable for corpus runtime
proof. This keeps the corpus evidence honest: a successful receipt means the
external source, not host fallback state, supplied the world, timeline, and
active runtime ownership.

## Music Source and Wall Ornament Ordinal Provider

`dm1_v1_f0740_f0743_music_source_pc34_compat` owns the DM1 music state
machine. It binds SONG.DAT at init, selects the F0742 map track on
stairs/teleporter transitions, and runs the F0743 update once per game
tick; F0740 pauses playback when the music toggle is off. M11 exposes
this only through the `dm1MusicSource`/`dm1MusicState`/`dm1MusicDriver`
fields — it does not itself decide track selection.

`dm1_v1_viewport_wall_ornament_ordinal_provider_pc34_compat.h` bridges
two independent ordinal sources into one
`DM1_ViewportWallOrnamentOrdinalCallback`: sensor-thing ornaments
(walking the thing list and reading `sensor.ornamentOrdinal`) are
resolved first, falling back to the F0169/F0170/F0171 random
wall/floor ornament computation keyed on `ornamentRandomSeed` and the
dungeon header's per-map random ornament counts. All 15 F0107 wall
ornament positions (center, side, and the `2` variants at each depth)
route through this single callback for both the M11 DM1 path and,
via a parallel raw-byte resolver, the CSB viewport path.

## Viewport Movement Completion Matrix

`tools/verify_dm1_v1_viewport_movement_completion_matrix.py` and
`parity-evidence/dm1_v1_viewport_movement_completion_matrix.md` track
proof of movement-driven viewport redraw across every depth/side
position. The matrix is now clear: `pass402` and `pass406` were the
last two open cells (D3L2/D3R2 wall ornament coverage), fixed alongside
the F0107 wall ornament wiring for Q-DM1-03.

## Original Transcript Capture Gates

Three companion gates cover the original-transcript debugging/capture
path:

- **Row preflight** (`pass625`) — validates a transcript row is
  well-formed before it is dispatched for turn redraw.
- **Turn redraw route** (`pass626`) — the source-anchored path a
  validated row takes to trigger viewport redraw on a turn.
- **Live debugger row gate** (`pass1075`) — gates live-debugger
  consumption of a transcript row on the same preflight/redraw
  invariants, preventing an unverified row from driving on-screen
  state.

None of these gates promote a synthetic transcript to stand in for a
real original capture; they only prove the handling path is correct
once a row is admitted.

## Inventory Modules

106 inventory modules implement the C05-C13 interaction matrix: chest
open/close and alcove placement, pickup/drop, stack split/merge,
scroll-wheel cycling, encumbrance and capacity limits, slot placement,
hand swap, and cross-champion transfers. Champion panel material for
inventory rendering shares the F0341/F0342 scroll draw path and the
F0115/G0209 object selector described in [DM1 Technical
Reference](DM1-Technical-Reference). Food and potion consumption is
fully implemented against all 10 potion formulas and 8 food amounts
(F0349); armour defense is computed on demand via F0143 during combat
rather than cached.

## Automap and Minimap

DM1 V1 automap state is owned by `dm1_v1_automap_pc34_compat.h`. Two
minimap presentation layers exist: `dm1_v1_minimap_pc34_compat.h` for
the V1 PC34-compatible surface, and `dm1_v2_minimap.h` /
`dm1_v2_minimap_pc34.h` for the V2 presentation-layer rendering of the
same source-owned map state (a parallel `csb_v2_minimap.h` serves CSB).
As with all V2 surfaces, these are display pipelines only and must not
alter the underlying dungeon-owned automap data.

## Focused Verification

```bash
./build/test_dm1_v1_group_visible_distance_pc34_compat
./build/test_dm1_v1_square_state_dispatch_pc34_compat
./build/test_dm1_v1_projectile_explosion_render_pc34_compat
./build/test_dm1_v1_original_save_pc34_handoff
```

Use a real PC34 data pair for final presentation capture. Headless/unit proof
does not establish Mac window composition, timing, or asset availability.
