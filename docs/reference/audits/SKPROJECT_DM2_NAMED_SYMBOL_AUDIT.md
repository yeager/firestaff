# skproject DM2 Named Symbol Audit

## Scope

This line-addressable inventory covers callable definitions in the
project-owned `.cpp` files under skproject `SKWIN/` and `SKULLWIN/`, excluding
bundled third-party SDK trees, plus the selected `DM2_ARRANGE_DUNGEON` row in
`SKWINSPX/src/v5/skmap.cpp`. Its source authority is the upstream revision
locked by `docs/parity/DM2_V1_PHASE0_PROVENANCE_GATE.md`:

- Repository: <https://github.com/gbsphenx/skproject>
- Locked commit: `2c59a2c5a7978923ff5c822c15c3479e4d448493`

The companion `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` is the machine-readable
ledger, with one row per named definition. Each row records a Firestaff
candidate, a disposition, and notes. An implementation disposition may cover
a narrow function slice and does not by itself prove integration into a
playable game route or parity on every supported platform.

This is not yet a complete functional audit of callable skproject source. The
repository README identifies `SKWINSPX` as its newer unified Windows/Linux/DOS
engine; the disposition ledger below covers only the older `SKWIN`/`SKULLWIN`
trees, with one selected SPX definition. `SKWINDOS` is also outside that
ledger. The supplemental
`SKPROJECT_DM2_ACTIVE_SOURCE_INVENTORY.tsv` lists per-file `ctags -x`
definitions from `SKWINSPX/src`, `SKWINDOS/src`, the legacy DOS files at
the top level of `SKWINDOS/`, and two `SKULLWIN/` definitions used in
cross-variant checks: 3,934 definition rows, 3,632 distinct names, and 302
repeated-name definitions. With active-source review overrides applied, the inventory currently maps 3,501 as `UNREVIEWED`, 373 as `PARTIAL`, and 50 as `IMPLEMENTED_PARITY`. Eight source entries are `NOT_APPLICABLE_ARCH`. The source-specific rows split into 1,118 SPX v4 definitions (980 unreviewed, 124 partial, 13 implemented, 1 architecture-specific), 1,347 SPX v5 definitions (1,224 unreviewed, 104 partial, 19 implemented), 262 shared SKWINSPX definitions (248 unreviewed, 5 partial, 7 architecture-specific, 2 outside-game), 832 legacy DOS definitions (716 unreviewed, 112 partial, 4 implemented), 373 SKWINDOS/src definitions (333 unreviewed, 26 partial, 14 implemented), and 2 SKULLWIN cross-variant definitions (both partial). The source-review ledger has 425 entries: 372 partial, 50 implemented, and 3 architecture-specific, including eight exact SKULLWIN cross-variant rows applied to matching named-symbol entries. Exact name overlap with the older
ledger is only a search hint, not proof of semantic parity. Editor and asset-tool trees (`DM2GDED`,
`DMDC2`, and `Utils`) are excluded because they do not implement the game
runtime.

## Inventory

| Source set | Definitions | Result |
|---|---:|---|
| Project-owned `.cpp` in `SKWIN/` and `SKULLWIN/`, plus selected `DM2_ARRANGE_DUNGEON` | 1,762 | Callable-definition rows in the disposition ledger |
| `SKWINSPX/src`, `SKWINDOS/src`, legacy top-level `SKWINDOS/`, and two `SKULLWIN/` definitions | 3,934 (2,727 SPX, 1,205 DOS, 2 SKULLWIN) | 3,501 `UNREVIEWED`; 373 `PARTIAL`; 50 `IMPLEMENTED_PARITY`; 8 `NOT_APPLICABLE_ARCH`; 2 `NOT_APPLICABLE_GAME` |

The active-source definitions are tracked per location and source variant,
not deduplicated by symbol name. The status split above keeps SPX v4 and SPX v5
separate; both the unified source root and legacy DOS tree remain visible as
independent review lanes. These are callable-definition counts, not the number
of completed product features or platform-complete behaviors.

## Dispositions

| Status | Count |
|---|---:|
| `IMPLEMENTED_PARITY` | 1,054 |
| `NOT_APPLICABLE_ARCH` | 558 |
| `NONAPPLICABLE` | 73 |
| `UNOWNED_SOURCE_MUTATION` | 0 |
| `PARTIAL` | 75 |
| `STATIC_INPUT_ONLY` | 0 |
| `PARTIAL_SOURCE_OWNER` | 2 |
| `MISSING` | 0 |
| **Total** | **1,762** |

The supplemental inventory covers 17 SPX v5 and 15 DOS `c_alloc.cpp`
definitions, SPX v4 `ACTUATE_DOOR` and `STEP_DOOR`, and legacy top-level DOS source. Reviewed entries are recorded in the companion source-review ledger with source mappings and variant-specific evidence. The supplemental review now covers wall/floor/pitfall/teleporter/trickwall actuator timer variants in legacy DOS and SKWINSPX v4/v5. Wall/floor mecha have live runtime owners; GAME_LOAD is narrower. Pitfall, teleporter and trickwall timer classes remain fail-closed in the active runtime dispatcher, and their standalone parity helpers do not implement the full source effects. The DOS
`DM2_ALLOC_MEMORY_RAM` body also ignores the clean flag and always clears
the allocated range, unlike the SPX branch. These source variants still
lack a live Firestaff allocator owner.

With active-source review overrides, the active inventory currently has 3,501 `UNREVIEWED`, 373 `PARTIAL`, and 50 `IMPLEMENTED_PARITY` definitions. There are eight architecture-specific source entries and two Bloodwych-only v6 rows excluded as `NOT_APPLICABLE_GAME`; it is not yet a feature-by-platform audit. The per-variant counts above show that SPX v4, SPX v5 and DOS still have large independently open definition sets. Reviewed actuator/projectile paths still have their specific gaps in the supplemental review. The legacy DOS shooter definitions in `SKWINDOS/dm2byg.cpp` and `dm2byc.cpp` are recorded as partial; their implementations do not have a complete live projectile owner. The DOS `SKW_IS_REBIRTH_ALTAR` predicate is also mapped to Firestaff’s matching helper; map-header input decoding and caller parity remain open for each supported platform. `SKW_QUERY_DOOR_STRENGTH` is source-compared to the callback helper. SPX v4 and v5 door-query variants are now separately reviewed. DOS/v5 strength reads field 0x11 with the field-0x10 fallback; SPX v4 reads field 0x0f directly. SPX v4 resistance reads field 0x0e and gets graphics from map-header flags/door types, while DOS/v5 use GDAT fields 0x0f/0x0d. Firestaff's map-door metadata path already selects parsed door sets behind header enable flags and door-record type; exact 0xff no-door consumer behavior and complete caller coverage remain open. Four Firestaff query receipts were corrected to match the locked skproject field indices; per-platform asset and runtime verification remain open. DOS, SPX v4 and SPX v5 `_2759_0155` variants now have source-specific review rows matched against the Firestaff query receipt: scan text entries 8–11 for a loadable entry, a nonzero command field 2 and nonzero charge field 8. DOS, SPX v4 and SPX v5 max-charge helpers also match the shared Firestaff DB5/DB6=15, DB10=3 mapping and null/unsupported=0 results; live item-stat-bar routing remains open. DOS/SPX v4/v5 container-map predicates now map to the DB9/type-1 Firestaff helper and runtime record-pool callback; non-hand-action platform routes still need coverage. The SPX-v4 `ADD_ITEM_TO_PLAYER` auto-equip order is now represented by a dedicated Firestaff adapter using the exact source slot groups. The adapter has no live DM2 runtime caller, so revive/hand workflows and platform routes remain open. `PUT_ITEM_TO_PLAYER` has a matching Firestaff helper and input-event dispatcher, but the runtime callback owner is not established. DOS, SPX v4 and SPX v5 `SELECT_CHAMPION_LEADER` all match the Firestaff leader-state helper for dead-target rejection, old/new flags and the last-hero exclusion; their platform-specific runtime routes remain open. SPX-v4 `ADD_MINION_ASSOC`/`RECOVER_MINION_ASSOC` stage temporary DB9/DB14 handles and reconstruct the serialized circular object-link graph; Firestaff's persistent `DM2_MinionTable` stores original object/owner pairs and does not implement that save/load rebinding path. DOS, SPX v4 and SPX v5 `APPEND_RECORD_TO` use the same parent-list, existing-tile chain, and new ground-root insertion semantics; Firestaff has bounded pool and dungeon-data helpers, but its runtime ownership and source fatal-error paths remain incomplete. Record-class lookup, live GDAT/command-string ownership and charge-stat-bar integration remain open. DOS/SPX v5 door attacks share a narrow Firestaff gate receipt that has no live caller; SPX v4 additionally supports weak-door override and compares damage against map-selected door resistance, which Firestaff does not yet model in an integrated owner.

The SKULLWIN, SPX v4 and SPX v5 general `FIND_WALK_PATH` implementations remain
partial. Firestaff's receipt initializer is empty, its live callback owner uses
a bounded same-map breadth-first route to the party, and the light walker is a
separate scored traversal. Those do not reproduce the source multi-map distance
maps, action-specific seeds/flags, or full path-selection behavior.

The SKULLWIN shooter audit also follows live callers: `DM2_ACTIVATE_SHOOTER` reaches a fail-closed helper before projectile creation. The active CCM callback supports only party attack and walking; although the source dispatch matrix admits `SHOOT_ITEM` states 0x0e/0x0f, Firestaff currently returns without an item shot. The source shooter consumes the active `c_creature` scratch fields `ItemToThrow`, direction, and facing, plus the timer tile, AIDef strength/Defense, and the creature possession chain. Firestaff's selection/damage helper does not have that live state owner or connect possession mutation to DB14/timer creation. `CREATURE_CAN_HANDLE_IT` also remains partial: Firestaff has a bounded GDAT mask-text parser and a pure 64-bit membership helper, while the live source constructs/caches a 64-byte mask from ability byte@4 and a DDAT variant, extracts item classes through DB-specific record and distinctive-item branches, and applies moneybox/open-state rules. The missing runtime chain and platform behavior are recorded in the supplemental review and named-symbol ledgers. These features remain partial, not parity-complete. Matching DOS implementations are in legacy top-level `SKWINDOS/` and remain individually audited.

The older allocator rows were rechecked against the current Firestaff call graph. The 16 `SKULLWIN/c_alloc.cpp` entries previously marked `IMPLEMENTED_PARITY` now remain `PARTIAL`: their mapped `dm2_v1_skproject_*_classify` routines produce classifier receipts or mutate a model, but no live Firestaff allocator owner calls them. This does not prove the source free-pool allocation, pointer movement, setup/teardown, and runtime behavior. The analogous `SKWINSPX` and `SKWINDOS` definitions remain individually unreviewed in the supplemental inventory.

The DOS, SPX v4 and SPX v5 `ALLOC_NEW_DBITEM` functions map item-spec DB/type bits to record allocation and initialize the type. Their `ALLOC_NEW_DBITEM_DEEPLY` counterparts first call `RECYCLE_A_RECORD_FROM_THE_WORLD`, then allocate on a miss, clear the full DB record, set the end marker and initialize type. Firestaff maps both narrow paths; its world recycler covers selected non-important ground/creature candidates and DB7/DB9 direct-return cases, while full source scanning and general deep-allocation ownership remain incomplete. All six active source rows remain partial.

The SKWINSPX v6 `ADD_PARTY_CHAMPION` command selects an arbitrary GDAT champion, revives it at the party, assigns party position/direction, sets the alive flag, and selects it as leader. Its nested selector rejects a held hand object or a full roster, but the outer function ignores the selector result and continues the lifecycle. Firestaff has bounded source-mirror and revive receipts, but no production owner for this v6 roster and interface mutation; the source row remains partial.

The DOS, SPX v4, SPX v5, SKWIN, and SKULLWIN creature allocators are now source-compared as separate variants. They allocate DB4 records and derive randomized hit points from the creature AI definition; high-bit creature types also attach a DB14 missile record. DOS/v5/SKULLWIN use `(min(multiplier, 31) + 1) * BaseHP >> 3`, while SPX v4/SKWIN use `min(multiplier, 31) * BaseHP >> 3`; record packing and cleanup behavior also differ. Firestaff only owns allocator behavior for timer 0x5e fixed inputs; the general spawn helper is test-only and disabled in production. All source variants remain partial, with generic caller behavior and DOS, Amiga, FM Towns, and Macintosh runtime ownership open.

The tail of the inventory includes `DRAW_ALCOVE_ITEMS` and
`DRAW_WALL_ORNATE`, both still partial, plus the partial
`_32cb_0f82_SHOP_GLASS`. In particular, the isolated
`GET_WALL_TILE_ANYITEM_RECORD` helper is not an implementation of
`DRAW_ALCOVE_ITEMS`: skproject reads each rendered wall cell's `id4` record
handle, applies an early facing gate, renders DB5..DB10 items with wall-specific
`DRAW_ITEM` geometry, handles two cloud types, and creates the clickable item
zone only for front cell 3. Firestaff now retains a bounded first DB4+ record
root on each eligible projected wall square, following the `w4=id4` handoff
from `SUMMARIZE_STONE_ROOM` before and independently of the `SeeThruWalls`
front-cell ornament gate; it still has no wall-cell item material/placement
pass, matching cloud route, or front-wall click target. Its wall-item geometry
cannot reuse the floor static-object plan: `DRAW_ITEM(tt=1)` starts from the
drawn wall picture's `w28/w30` anchor and `rectNo`, adds the item's signed
`dtImageOffset[0xFD]` and the slot delta from `_4976_41b0/_4976_41de`. The later
`vv != -1` block applies that slot delta again, then adds the selected
image-field `dtImageOffset` before `QUERY_TEMP_PICST`; that call
adds the generic `0xFE` offset and selected-field offset to the image
metadata as well. `QUERY_TEMP_PICST` scales the combined anchor with
`_4976_418e[y_distance][1]` and clips through the wall ornament rectangle
with reference point zero. SKWINSPX v4 further confirms that
`DRAW_ALCOVE_ITEMS` writes `_4976_41b0[slot][0] = 0` for indices 0..31 before
drawing, then invokes `DRAW_ITEM` with slots 2..13. The shared table is declared
`[16][2]`, so that SPX initialization loop exceeds its declared rows; only
the values used by item slots 2..13 are in bounds. For those slots it removes
the second x displacement, while `DRAW_ITEM(tt=1)` applies the y displacement
in both its initial anchor and the later `vv != -1` block (`ref == NULL`). This
is an SPX-specific source detail that must be reconciled with primary SKWIN,
which does not show the reset loop, before claiming cross-build parity. The
wall anchor is the separate `glbDrawExtPicture` (`_4976_5aa8`)
snapshot; the tracked SKWIN and SKWINSPX sources read that owner but do not show
its population in a named helper, so a runtime bind still needs an audited
producer before it can claim source placement. The floor plan's cell pass,
5x5 anchor, and one-pass slot offsets therefore cannot substitute. Preserve
the early facing mismatch return before
the DB-type check, the DB5..DB10-only draw-slot advancement (2 through 13,
then wrap to 2), and the cell-3-only clickable zone. The next work is to
consume that root with the source facing gate, this wall-specific placement,
and the corresponding clipping. The shop-glass routine now resolves the live wall/floor actuator pair and begins
its mask/whitelist/ownership transition in source byte order. The separate
`ddat.v1d67bc` hand-container owner, item name/prices, DB14 allocation and
buy/sell remain open. The selected merchandise image, conditional sold-out
icon and nav board use their source GDAT images, palettes, offsets and clipped
rectangles on the live render route; incomplete ownership still fails closed.

Open source-owned behavior includes the
`DM2_ALLOC_NEW_RECORD` / `DM2_ALLOC_NEW_DBITEM` /
`DM2_ALLOC_NEW_DBITEM_DEEPLY` chain: Firestaff has the pool scan and limited
recycler routes, but not the complete source recycler across databases, map
cursor, protected maps, timers and CAII ownership. `DM2_DEALLOC_RECORD` itself is implemented at source semantics: the bounded
record-pool owner changes only the first word to `OBJECT_NULL` using source
byte order, with a regression that checks the untouched payload bytes. Other
game-level callers remain open where their list cuts, timers, CAII and owner
transactions are incomplete. `DM2_ARRANGE_DUNGEON`
has a bounded source-owned mutation path integrated into new-game loading, but
still lacks source-by-source mutation and all-edition/platform parity proof.
`DM2_ADD_BACKGROUND_LIGHT_FROM_TILE` and
`DM2_RECYCLE_A_RECORD_FROM_THE_WORLD` remain partial. See each TSV row's notes
before treating a mapping as a parity result.

`NOT_APPLICABLE_ARCH` and `NONAPPLICABLE` are separate dispositions: neither
claims that a supported DM2 game feature is implemented. The 1,762 callable-definition
rows cover the older `SKWIN`/`SKULLWIN` source sets; the expanded supplemental
inventory contains 3,932 `SKWINSPX`/`SKWINDOS` definitions, with 3,656
`UNREVIEWED`, 255 `PARTIAL`, and 21 `IMPLEMENTED_PARITY` definitions after
active-source review overrides. Per-function comparison and runtime evidence remain required for
DOS, Amiga, FM Towns, and Macintosh.

## Machine check

Regenerate the supplemental source discovery list from the locked skproject
checkout with `ctags` installed:

```sh
python3 tools/generate_skproject_active_source_inventory.py \
  --source-root /path/to/skproject
```

The generator rejects a checkout whose `HEAD` differs from the locked commit.
The resulting names and locations are still discovery evidence; review each
row's behavior and Firestaff caller before changing its disposition.

The open queue combines the disposition ledger with the unreviewed source
definitions:

```sh
python3 tools/symbol_backlog.py --game DM2 --reference skproject
```

At this revision it reports 3,951 open rows: 3,529 unreviewed active-source
definitions, 420 partial source or older-ledger rows, and 2 partial source-owner
rows. Eight source entries are architecture-specific and two Bloodwych-only definitions are excluded from the requested three-game queue. Recounted with `python3 tools/symbol_backlog.py --game DM2 --reference skproject`. DOS, SPX v4/v5, and SKULLWIN `CHANGE_PLAYER_POS` definitions are source-compared; M11 now calls the source-shaped two-stage formation model, while cursor-icon feedback and non-M11 platform routes remain open. DOS and SPX v5 `ADJUST_HAND_COOLDOWN` match the shared arithmetic helper, while SPX v4 has a separate source-faithful helper because it uses an already-computed delay and Aura of Speed. All three definitions remain partial because there is no production caller/platform binding. `ADJUST_SKILLS` now uses source random-bit masks, priest/wizard maxMP formulas, final-level-based maxHP rolls, and class-scaled stamina rolls for DOS/SPX v5/SKULLWIN; this corrected helper still has no production caller. SPX v4 now has a separate helper for its 25-tick combat-double window and current-stamina base; its map-header, RNG, UI and production platform bindings remain open, so all four source definitions stay partial. The expanded review now records separate SPX v4/v5 `CREATURE_CAN_HANDLE_IT`
rows, the SPX v4 creature-shooter route, and source-matched partial rows for
`DM2_CHECK_RECOMPUTE_LIGHT` across SKULLWIN and SPX v5; these remain partial because the live
AI mask/record owner and possession/projectile lifecycle are not integrated. DOS, SPX v4, and SPX v5 stamina-adjustment definitions also remain partial: Firestaff matches the helper arithmetic, and the live movement owner applies source stamina mutation and pending wounds from authenticated party/weight inputs. The helper itself has no production caller, and other stamina callers remain unbound. `ACTIVATE_ACTION_HAND` and `SET_SPELLING_CHAMPION` are source-compared for
DOS, SPX v4, and SPX v5; each
variant remains partial because M11 redraw and the separate rune panel provide visible equivalents for the current hand/spell selection, while pending-position recompute and source-specific transient UI consumers are not mapped. DOS and SPX v4/v5 coin-wallet definitions now have separate source review
rows because their add non-currency return behavior differs; take-wallet
variants also map to Firestaff's shared helper. Runtime platform selection
remains open. The active inventory
now includes the legacy DOS implementation files. Exact-name overlap is shown only as a
candidate for source comparison.

```sh
awk -F '\t' 'NR > 1 { count[$6]++; rows++ }
  END { print rows, count["IMPLEMENTED_PARITY"], count["NOT_APPLICABLE_ARCH"],
              count["NONAPPLICABLE"], count["UNOWNED_SOURCE_MUTATION"],
              count["PARTIAL"], count["STATIC_INPUT_ONLY"],
              count["PARTIAL_SOURCE_OWNER"] }' \
  docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv
```

SPX v4 `ADD_CACHE_HASH` and the older SKWIN definition share sorted hash lookup, duplicate-index return and insertion ordering with Firestaff’s bounded cache model. Cache-maintenance on tick changes and mement retrieval/touch on duplicate hits are not represented, and supported runtime cache owners remain open.

DM2 `ADJUST_UI_EVENT` variants diverge at event codes 95-98. DOS, SPX v4 and SKWIN treat them as spell/leader panel clicks and use rectangle geometry plus the leader-state gate; SPX v5 and SKULLWIN treat them as movement arrows and use the cooldown-2 movement gate. Firestaff’s shared helper implements the SPX-v5 arrow behavior, so the DOS/SPX-v4/SKWIN definitions are partial; the SPX-v5 core mapping matches, but a live platform input owner remains open.

SPX v4 and SKWIN `GET_TEMP_CACHE_HASH` mutate the global temporary-hash counter for every collision probe; Firestaff now matches that bounded 16-bit counter progression in its helper and allocator. Firestaff retains explicit bounds for all-collisions and cache-full cases, while source allocation failure behavior and runtime cache ownership remain open. `FREE_TEMP_CACHE_INDEX` frees the cache index when no mement is attached before freeing the indexed mement; Firestaff currently returns early without clearing that cache entry. These cache functions remain partial pending remaining source-side effects and runtime-owner evidence.


## Scope correction: Bloodwych v6 creature-name helper (2026-11-01)

`SKWINSPX/src/v6/xcorebwy.cpp:100` `BW_GetCreatureName` maps Bloodwych-specific creature IDs to names and is used by the Bloodwych import diagnostic at line 1904. The source file identifies itself as experimental Bloodwych dungeon-reading code. Classified `NOT_APPLICABLE_GAME`; this is not a DM2 feature to reproduce. The active-source inventory still has 3,529 unreviewed, 347 partial, 49 implemented-parity, 8 architecture-specific and now 2 outside-game definitions. The generated DM2 backlog remains 3,950 open rows (3,528 unreviewed, 420 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 collision-wound source pass (2026-11-01)

Compared DOS `SKW_12b4_023f` (`dm2bya.cpp:129`), SPX v4 `_12b4_023f_WOUND_RUNNING_INTO_CREATURE` (`skcore.cpp:6235`), and SPX v5 `DM2_move_12b4_023f` (`skmove.cpp:117`) with Firestaff's narrow `dm2_v1_skproject_move_12b4_023f` receipt helper and the SKULLWIN counterpart. The direction candidates, duplicate suppression, wound-result accounting, and success-triggered noise requests are represented. Firestaff still lacks production candidate-position lookup, live WOUND_PLAYER mutation, noise scheduling/playback and source return behavior; all three source definitions are `PARTIAL`. Current DM2 active-source counts: 3,525 unreviewed, 350 partial, 49 implemented-parity, 8 architecture-specific, and 2 outside-game. The combined open backlog remains 3,950 (3,525 unreviewed, 423 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 side-offset movement source pass (2026-11-01)

Compared DOS `SKW_12b4_0953` (`dm2bya.cpp:144`), SPX v4 `CREATURE_12b4_0953` (`skcrture.cpp:6961`), and SPX v5 `DM2_12b4_0953` (`skmove.cpp:32`) with Firestaff's `dm2_v1_skproject_move_side_offset` and its use in staged GAME_LOAD. The helper matches the facing bits, side-relative direction filter, and nonzero 5x5 horizontal offset. Live `dm2_v1_runtime_move` still treats an occupied destination as a blocked creature step and does not use the source C5/C4 side-offset result; all three definitions remain `PARTIAL`. Current DM2 counts: 3,522 unreviewed, 353 partial, 49 implemented-parity, 8 architecture-specific, and 2 outside-game. Combined open backlog remains 3,950 (3,522 unreviewed, 426 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 new-map loader source pass (2026-11-01)

Compared DOS `SKW_LOAD_NEWMAP` (`dm2bya.cpp:153`), SPX v4 `LOAD_NEWMAP` (`skcore.cpp:2783`), and SPX v5 `DM2_LOAD_NEWMAP` (`sklodlvl.cpp:1337`) with Firestaff's `dm2_v1_load_newmap` adapter. The callback adapter represents the phase order for cursor state, optional level-exit/entry actuators, graphics table, CAII fill, viewport dirty state, light recomputation, optional event and cursor restore. Firestaff's dynamic-level loader refuses work without a complete live map/record/GDAT owner, and the orchestrator is called only by its test; all three source definitions remain `PARTIAL`. Active-source counts: 3,519 unreviewed, 356 partial, 49 implemented-parity, 8 architecture-specific and 2 outside-game. Combined DM2 backlog remains 3,950 (3,519 unreviewed, 429 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 local-creature-context source pass (2026-11-01)

Compared DOS `SKW_13e4_0004` (`dm2bya.cpp:179`), SPX v4 `PREPARE_LOCAL_CREATURE_VAR` (`skgame.cpp:3018`), and SPX v5 `DM2_PREPARE_LOCAL_CREATURE_VAR` (`skai.cpp:5824`) with Firestaff's creature-AI context adapter. Firestaff records context fields and classifies selected branches, but its preparation function returns NULL/fail-closed and does not save or initialize the live s350, DB4, CAII, AI-spec, timer, or version-specific global owners. All three source definitions remain `PARTIAL`. Active-source counts: 3,516 unreviewed, 359 partial, 49 implemented-parity, 8 architecture-specific, 2 outside-game. Combined backlog remains 3,950 (3,516 unreviewed, 432 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 creature-AI initialization source pass (2026-11-01)

Compared DOS `SKW_13e4_01a3` (`dm2bya.cpp:227`), SPX v4 `_13e4_01a3` (`skcrture.cpp:3910`), and SPX v5 `DM2_13e4_01a3` (`skai.cpp:2347`) with Firestaff's `dm2_v1_init_creature_ai_state`. Firestaff documents the s350 field, AI-spec/GDAT, timing/RNG and allocation-fallback contract, but the initializer fails closed without running it. The v4 global layout and timer/cache path also differ. All three source definitions remain `PARTIAL`. Active-source counts: 3,513 unreviewed, 362 partial, 49 implemented-parity, 8 architecture-specific and 2 outside-game. Combined DM2 backlog remains 3,950 (3,513 unreviewed, 435 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 command/AI-turn source pass (2026-11-01)

Compared DOS `SKW_13e4_0360` (`dm2bya.cpp:270`), SPX v4 `CREATURE_SET_NEW_COMMAND` (`skcrture.cpp:400`), and SPX v5 `DM2_ai_13e4_0360` (`skai.cpp:5919`). The v5 function's guards, CAII byte writes, command-0x13 protection, and timer-cancel/requeue tail match Firestaff's `dm2_v1_caii_ai_13e4_0360`, which has production runtime callers; mark the v5 definition `IMPLEMENTED_PARITY`. DOS uses a different CAII layout that is not source-mapped to Firestaff's slot offsets, and v4 implements a distinct Command2/sequence-control operation with no direct Firestaff setter; keep those two `PARTIAL`. Current DM2 inventory: 3,510 unreviewed, 364 partial, 50 implemented-parity, 8 architecture-specific and 2 outside-game. Combined backlog: 3,949 open (3,510 unreviewed, 437 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 creature-animation timing source pass (2026-11-01)

Compared DOS `SKW_13e4_071b` (`dm2bya.cpp:301`), SPX v4 `CREATURE_13e4_071b` (`skcrture.cpp:5297`), and SPX v5 `DM2_ai_13e4_071b` (`skai.cpp:5969`). They align the creature animation phase to the game tick, set a completion marker on a zero remainder, or replace the creature timer with a frame-count-based delay. Firestaff has a classification receipt and documents the contract, but its runtime timing helper fails closed and no production AI timing callback mutates animation state or owns the timer queue. All three variants remain `PARTIAL`. Current DM2 counts: 3,507 unreviewed, 367 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; 3,949 backlog rows (3,507 unreviewed, 440 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 creature-animation 0x2000 timing source pass (2026-11-01)

Compared DOS `SKW_13e4_0806` (`dm2bya.cpp:323`), SPX v4 `CREATURE_13e4_0806` (`skcrture.cpp:5328`), and SPX v5 `DM2_ai_13e4_0806` (`skai.cpp:6008`). All check a stable-animation guard, compute the tick/frame-count remainder, write completion or 0xa000 timing flags, and replace the creature timer when the remainder is nonzero. Firestaff's classifier represents the decision, but its timing helper fails closed before animation mutation or timer operations, and no production AI timing callback is bound. All three remain `PARTIAL`. Current counts: 3,504 unreviewed, 370 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; backlog 3,949 (3,504 unreviewed, 443 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 AI action-context source pass (2026-11-01)

Compared DOS `SKW_14cd_0276` (`dm2bya.cpp:370`), SPX v4 `CREATURE_PATH_14cd_0276` (`skcrture.cpp:5783`), and SPX v5 `DM2_14cd_0276` (`skai.cpp:4183`). Firestaff extracts the behavior fields and allocation predicate, while the sources also set live AI context/pointer fields, allocate or reuse path storage, and copy creature walk-path bytes. The Firestaff adapter performs no allocation or live-context update and has no production caller. All three variants remain `PARTIAL`. Counts: 3,501 unreviewed, 373 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; DM2 backlog 3,949 (3,501 unreviewed, 446 partial source/older-ledger rows, 2 partial source owners).
