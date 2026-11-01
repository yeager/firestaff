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
repeated-name definitions. With active-source review overrides applied, the inventory currently maps 3,360 as `UNREVIEWED`, 506 as `PARTIAL`, 5 as `MISSING`, and 53 as `IMPLEMENTED_PARITY`. Eight source entries are `NOT_APPLICABLE_ARCH`. The source-specific rows split into 1,118 SPX v4 definitions (936 unreviewed, 166 partial, 1 missing, 14 implemented, 1 architecture-specific), 1,347 SPX v5 definitions (1,176 unreviewed, 149 partial, 2 missing, 20 implemented), 262 shared SKWINSPX definitions (248 unreviewed, 5 partial, 7 architecture-specific, 2 outside-game), 832 legacy DOS definitions (667 unreviewed, 158 partial, 2 missing, 5 implemented), 373 SKWINDOS/src definitions (333 unreviewed, 26 partial, 14 implemented), and 2 SKULLWIN cross-variant definitions (both partial). The source-review ledger has 582 entries: 513 partial, 5 missing, 54 implemented, 8 architecture-specific, and 2 outside-game rows, including eight exact SKULLWIN cross-variant rows applied to matching named-symbol entries. Exact name overlap with the older
ledger is only a search hint, not proof of semantic parity. Editor and asset-tool trees (`DM2GDED`,
`DMDC2`, and `Utils`) are excluded because they do not implement the game
runtime.

## Inventory

| Source set | Definitions | Result |
|---|---:|---|
| Project-owned `.cpp` in `SKWIN/` and `SKULLWIN/`, plus selected `DM2_ARRANGE_DUNGEON` | 1,762 | Callable-definition rows in the disposition ledger |
| `SKWINSPX/src`, `SKWINDOS/src`, legacy top-level `SKWINDOS/`, and two `SKULLWIN/` definitions | 3,934 (2,727 SPX, 1,205 DOS, 2 SKULLWIN) | 3,407 `UNREVIEWED`; 464 `PARTIAL`; 3 `MISSING`; 50 `IMPLEMENTED_PARITY`; 8 `NOT_APPLICABLE_ARCH`; 2 `NOT_APPLICABLE_GAME` |

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

With active-source review overrides, the active inventory currently has 3,360 `UNREVIEWED`, 506 `PARTIAL`, 5 `MISSING`, and 53 `IMPLEMENTED_PARITY` definitions. There are eight architecture-specific source entries and two Bloodwych-only v6 rows excluded as `NOT_APPLICABLE_GAME`; it is not yet a feature-by-platform audit. The per-variant counts above show that SPX v4, SPX v5 and DOS still have large independently open definition sets. Reviewed actuator/projectile paths still have their specific gaps in the supplemental review. The legacy DOS shooter definitions in `SKWINDOS/dm2byg.cpp` and `dm2byc.cpp` are recorded as partial; their implementations do not have a complete live projectile owner. The DOS `SKW_IS_REBIRTH_ALTAR` predicate is also mapped to Firestaff’s matching helper; map-header input decoding and caller parity remain open for each supported platform. `SKW_QUERY_DOOR_STRENGTH` is source-compared to the callback helper. SPX v4 and v5 door-query variants are now separately reviewed. DOS/v5 strength reads field 0x11 with the field-0x10 fallback; SPX v4 reads field 0x0f directly. SPX v4 resistance reads field 0x0e and gets graphics from map-header flags/door types, while DOS/v5 use GDAT fields 0x0f/0x0d. Firestaff's map-door metadata path already selects parsed door sets behind header enable flags and door-record type; exact 0xff no-door consumer behavior and complete caller coverage remain open. Four Firestaff query receipts were corrected to match the locked skproject field indices; per-platform asset and runtime verification remain open. DOS, SPX v4 and SPX v5 `_2759_0155` variants now have source-specific review rows matched against the Firestaff query receipt: scan text entries 8–11 for a loadable entry, a nonzero command field 2 and nonzero charge field 8. DOS, SPX v4 and SPX v5 max-charge helpers also match the shared Firestaff DB5/DB6=15, DB10=3 mapping and null/unsupported=0 results; live item-stat-bar routing remains open. DOS/SPX v4/v5 container-map predicates now map to the DB9/type-1 Firestaff helper and runtime record-pool callback; non-hand-action platform routes still need coverage. The SPX-v4 `ADD_ITEM_TO_PLAYER` auto-equip order is now represented by a dedicated Firestaff adapter using the exact source slot groups. The adapter has no live DM2 runtime caller, so revive/hand workflows and platform routes remain open. `PUT_ITEM_TO_PLAYER` has a matching Firestaff helper and input-event dispatcher, but the runtime callback owner is not established. DOS, SPX v4 and SPX v5 `SELECT_CHAMPION_LEADER` all match the Firestaff leader-state helper for dead-target rejection, old/new flags and the last-hero exclusion; their platform-specific runtime routes remain open. SPX-v4 `ADD_MINION_ASSOC`/`RECOVER_MINION_ASSOC` stage temporary DB9/DB14 handles and reconstruct the serialized circular object-link graph; Firestaff's persistent `DM2_MinionTable` stores original object/owner pairs and does not implement that save/load rebinding path. DOS, SPX v4 and SPX v5 `APPEND_RECORD_TO` use the same parent-list, existing-tile chain, and new ground-root insertion semantics; Firestaff has bounded pool and dungeon-data helpers, but its runtime ownership and source fatal-error paths remain incomplete. Record-class lookup, live GDAT/command-string ownership and charge-stat-bar integration remain open. DOS/SPX v5 door attacks share a narrow Firestaff gate receipt that has no live caller; SPX v4 additionally supports weak-door override and compares damage against map-selected door resistance, which Firestaff does not yet model in an integrated owner.

The SKULLWIN, SPX v4 and SPX v5 general `FIND_WALK_PATH` implementations remain
partial. Firestaff's receipt initializer is empty, its live callback owner uses
a bounded same-map breadth-first route to the party, and the light walker is a
separate scored traversal. Those do not reproduce the source multi-map distance
maps, action-specific seeds/flags, or full path-selection behavior.

The SKULLWIN shooter audit also follows live callers: `DM2_ACTIVATE_SHOOTER` reaches a fail-closed helper before projectile creation. The active CCM callback supports only party attack and walking; although the source dispatch matrix admits `SHOOT_ITEM` states 0x0e/0x0f, Firestaff currently returns without an item shot. The source shooter consumes the active `c_creature` scratch fields `ItemToThrow`, direction, and facing, plus the timer tile, AIDef strength/Defense, and the creature possession chain. Firestaff's selection/damage helper does not have that live state owner or connect possession mutation to DB14/timer creation. `CREATURE_CAN_HANDLE_IT` also remains partial: Firestaff has a bounded GDAT mask-text parser and a pure 64-bit membership helper, while the live source constructs/caches a 64-byte mask from ability byte@4 and a DDAT variant, extracts item classes through DB-specific record and distinctive-item branches, and applies moneybox/open-state rules. The missing runtime chain and platform behavior are recorded in the supplemental review and named-symbol ledgers. These features remain partial, not parity-complete. Matching DOS implementations are in legacy top-level `SKWINDOS/` and remain individually audited.

Compared DOS `R_1FE74` (`SKWINDOS/dm2bya.cpp:1304`) and SPX v5 `R_1FE74` (`SKWINSPX/src/v5/unref.cpp:54`). Both implement the same signed 16-bit axis-alignment and direction predicate; the source labels it unreferenced. Firestaff has no matching predicate or call route, and SPX v4 has no separate definition. Both source rows are recorded `MISSING`. Current active-source counts: 3,360 unreviewed, 506 partial, 5 missing, 53 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,946 open rows (3,360 unreviewed, 579 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_1c9a_0cf7` and SPX v5 `DM2_1c9a_0cf7`. Both locate the creature at the supplied tile, replace its queued timer, choose event type 0x21/0x22 from its group link, enqueue for the next game tick, and store the ticket in the CAII slot. Firestaff has matching record-pool/timer helpers with live composition in missile-impact and world/load paths. Its public reschedule wrapper has no direct M11 caller, and SPX v4 uses a distinct timer owner without this same scheduling entrypoint; full caller and platform parity remain open. Both reviewed definitions are `PARTIAL`. Current active-source counts: 3,383 unreviewed, 483 partial, 5 missing, 53 implemented-parity, 8 architecture-specific, 2 outside-game; combined backlog: 3,946 (3,383 unreviewed, 556 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_1c9a_0db0`, SPX v4 `RELEASE_CREATURE_TIMER`, and SPX v5 `DM2_1c9a_0db0`. Each validates the creature record and CAII slot, cancels a pending timer, and clears the slot ticket. Firestaff implements the same bounded operation in `dm2_v1_caii_delete_timer`, composed into live scheduling, free-slot and creature-loop routes. Queue ownership and caller parity remain incomplete across platforms; all three source definitions are `PARTIAL`. Current active-source counts: 3,380 unreviewed, 486 partial, 5 missing, 53 implemented-parity, 8 architecture-specific, 2 outside-game; combined backlog: 3,946 (3,380 unreviewed, 559 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_1c9a_0fcb`, SPX v4 `CREATURE_1c9a_0fcb`, and SPX v5 `DM2_1c9a_0fcb`. All free the allocated creature slot and cancel its pending timer. DOS/v5 also conditionally delete a source creature based on AI flags, mode byte and saved timer coordinates; v4 has no such tail. Firestaff models release and the v5 decision head in `dm2_v1_caii_free_slot`, but omits the conditional record-delete mutation, has no production M11 caller for its public wrapper, and lacks source allocator recycling. All three definitions are `PARTIAL`. Current active-source counts: 3,377 unreviewed, 489 partial, 5 missing, 53 implemented-parity, 8 architecture-specific, 2 outside-game; combined backlog: 3,946 (3,377 unreviewed, 562 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_DELETE_CREATURE_RECORD`, SPX v4 `DELETE_CREATURE_RECORD`, and SPX v5 `DM2_DELETE_CREATURE_RECORD`. Firestaff has a live lethal-damage route and world/load owner for the bounded full deletion transaction: creature lookup, data-backed trigger gate, CAII cleanup, tile-chain cut, possession drops, and record deallocation. The 3CE7D move side effects, dballoc/GFX-cache cleanup, and full direct-caller/platform integration remain open; all three source definitions are `PARTIAL`. Current active-source counts: 3,374 unreviewed, 492 partial, 5 missing, 53 implemented-parity, 8 architecture-specific, 2 outside-game; combined backlog: 3,946 (3,374 unreviewed, 565 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_1c9a_17c7`, SPX v4 `_1c9a_17c7`, and SPX v5 `DM2_1c9a_17c7`. All implement the current-map and state gates, reject diagonal offsets, require party-facing vector direction, and cap each coordinate delta at two tiles. Firestaff has the exact predicate in `dm2_v1_skproject_1c9a_17c7`, but no production caller; `dm2_v1_1c9a_17c7` returns square distance and is not equivalent. WOUND_CREATURE integration and supported-platform routing remain open. All three definitions are `PARTIAL`. Current active-source counts: 3,371 unreviewed, 495 partial, 5 missing, 53 implemented-parity, 8 architecture-specific, 2 outside-game; combined backlog: 3,946 (3,371 unreviewed, 568 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_APPLY_CREATURE_POISON_RESISTANCE`, SPX v4 `APPLY_CREATURE_POISON_RESISTANCE`, and SPX v5 `DM2_APPLY_CREATURE_POISON_RESISTANCE`. The three use the same immunity and resistance-nibble formula; Firestaff implements that calculation in two helpers and binds it to runtime cloud damage. Exact random-stream ordering, every spell/attack caller, and all platform routes remain open. All three definitions are `PARTIAL`. Current active-source counts: 3,360 unreviewed, 506 partial, 5 missing, 53 implemented-parity, 8 architecture-specific, 2 outside-game; combined backlog: 3,946 (3,360 unreviewed, 579 partial source/older-ledger rows, 5 missing, 2 partial source owners).

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


## Latest DM2 target-validation source pass (2026-11-01)

Compared DOS `SKW_14cd_0389`, SPX v4 `CREATURE_THINK_0389`, and SPX v5 `DM2_14cd_0389` with Firestaff's target-validation receipts. Each source validates live AI context, dispatches the selected creature action through the version-specific 0f0a handler, checks the resulting plan count, and may update the creature's selected target/plan field. Firestaff records guard and table-byte information but does not execute the live handler or mutate the creature, and has no production AI caller. All three definitions remain `PARTIAL`. Current counts: 3,498 unreviewed, 376 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,498 unreviewed, 449 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 action-flags source pass (2026-11-01)

Compared DOS `SKW_14cd_062e`, SPX v4 `_14cd_062e`, and SPX v5 `DM2_14cd_062e` with Firestaff's action-flag helper. The helper reproduces the table byte@5 mask and cross-map clear rule for supplied table/map inputs, but it has no production caller; the skproject classifier leaves table-derived results unresolved. All three definitions remain `PARTIAL`. Current counts: 3,495 unreviewed, 379 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,495 unreviewed, 452 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 next-action source pass (2026-11-01)

Compared DOS `SKW_DECIDE_NEXT_XACT`, SPX v4 action-table behavior, and SPX v5 `DM2_DECIDE_NEXT_XACT`. Firestaff's table-walker helper covers supplied entries, including 0xf6 register writes, but the source classifier stops before reading table data and no production runtime caller uses the helper. The DOS and SPX definitions remain `PARTIAL`. Current counts: 3,492 unreviewed, 382 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,492 unreviewed, 455 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 post-action transition source pass (2026-11-01)

Compared DOS `SKW_14cd_08f5`, SPX v4 `CREATURE_THINK_08F5`, and SPX v5 `DM2_14cd_08f5`. Firestaff's helper computes the branch transition from supplied action-table rows, while the sources also clear or update live table/row state and return whether the action changed. The helper has no production AI-loop caller and does not mutate the live context. All three definitions remain `PARTIAL`. Current counts: 3,489 unreviewed, 385 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,489 unreviewed, 458 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 action-dispatch source pass (2026-11-01)

Compared DOS `SKW_14cd_0f0a`, SPX v4 `_14cd_0f0a_EXE_17`, and SPX v5 `DM2_14cd_0f0a`. The source dispatchers run one of 17 action bodies that mutate live AI/game state. Firestaff classifies the case but its dispatcher fails closed before invoking any action body. All three definitions remain `PARTIAL`. Current counts: 3,486 unreviewed, 388 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,486 unreviewed, 461 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 action-entry source pass (2026-11-01)

Compared DOS `SKW_14cd_0f3c`, SPX v4 `_14cd_0f3c`, and SPX v5 `DM2_ai_14cd_0f3c`. Firestaff's helper computes the strength and record fields, while the source appends the populated entry to the live AI action array and increments its count. The helper has no production caller and does not mutate that live array. All three definitions remain `PARTIAL`. Current counts: 3,483 unreviewed, 391 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,483 unreviewed, 464 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 hexe action-walk source pass (2026-11-01)

Compared DOS `SKW_14cd_18f2`, SPX v4 `_14cd_18f2`, and SPX v5 `DM2_14cd_18f2`. The sources scan live hexe entries, apply action-condition checks, adjust selected entries, and append derived actions. Firestaff has separate condition-walk and action-entry helpers, but no live list traversal or production binding. All three definitions remain `PARTIAL`. Current counts: 3,480 unreviewed, 394 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,480 unreviewed, 467 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 condition-gated action source pass (2026-11-01)

Compared DOS `SKW_14cd_19c2`, SPX v4 `CREATURE_14cd_19c2`, and SPX v5 `DM2_14cd_19c2`. These wrappers gate on readiness, evaluate a hexe condition, update live action flags, and delegate to the 18f2 action-list walk. Firestaff has separate condition and list-walk receipts, but lacks a production-bound composition and live state mutation. All three definitions remain `PARTIAL`. Current counts: 3,477 unreviewed, 397 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,477 unreviewed, 470 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 condition-table action source pass (2026-11-01)

Compared DOS `SKW_14cd_1a78`, SPX v4 `_14cd_1a78`, and SPX v5 `DM2_14cd_1a78`. Sources resolve action conditions, scan hexe entries, run per-entry predicates, and append derived actions. Firestaff records table matches but does not run the source predicate or mutate the live action array, and has no production caller. All three definitions remain `PARTIAL`. Current counts: 3,474 unreviewed, 400 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,474 unreviewed, 473 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 conditional spell-action source pass (2026-11-01)

Compared DOS `SKW_14cd_1bac`, SPX v4 `_14cd_1bac`, and SPX v5 `DM2_14cd_1bac`. The sources gate on readiness/attack mode, evaluate action conditions, update the live action mask, and delegate to the 18f2 walker. Firestaff records branch decisions but does not mutate the mask or call the walker in a production route. All three definitions remain `PARTIAL`. Current counts: 3,471 unreviewed, 403 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,471 unreviewed, 476 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 target-position action pass (2026-11-01)

Compared DOS `SKW_14cd_1c8d`, SPX v4 `_14cd_1c8d_PFN17_09`, and SPX v5 `DM2_14cd_1c8d`. Firestaff's classifier matches the coordinate/map gate, but the source delegates to the 18f2 action-list walker on nonmatch and Firestaff's action wrapper fails closed before doing so. All three definitions remain `PARTIAL`. Current counts: 3,468 unreviewed, 406 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,468 unreviewed, 479 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 minion-missile action source pass (2026-11-01)

Compared DOS `SKW_14cd_1cec`, SPX v4 `_14cd_1cec_PFN17_10`, and SPX v5 `DM2_14cd_1cec`. Each resolves a minion's missile reference, checks for missile record type 9, reads its action parameter, and delegates to the action-list builder. Firestaff's classifier covers the reference lookup but does not execute the action walk or bind the production route. All three definitions remain `PARTIAL`. Current counts: 3,465 unreviewed, 409 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,465 unreviewed, 482 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 action-table copy source pass (2026-11-01)

Compared DOS `SKW_14cd_1eec`, SPX v4 `_14cd_1eec`, and SPX v5 `DM2_14cd_1eec`. Sources scan matching hexe entries, apply the action predicate, copy/adjust entries, and append derived actions. Firestaff records table-walk shape but does not execute the live predicate/mutation path or bind production callers. All three definitions remain `PARTIAL`. Current counts: 3,462 unreviewed, 412 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,462 unreviewed, 485 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 75 source pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_75`, SPX v4 `PROCEED_XACT_75`, and SPX v5 `DM2_PROCEED_XACT_75`. Firestaff has a source-mapped cache/mode receipt, while the reference paths also mutate live creature/AI flags and invoke the 19f0_0891 transaction. Firestaff's XACT-use-item runtime helper fails closed and has no production platform route. All three definitions remain `PARTIAL`. Current counts: 3,459 unreviewed, 415 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,459 unreviewed, 488 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 random-turn XACT source pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_57`, SPX v4 `PROCEED_XACT_57`, and SPX v5 `DM2_PROCEED_XACT_57`. Firestaff's callback helper models the random first movement, alternate attempt, and turn fallback, but no production AI/XACT route calls it. All three source definitions remain `PARTIAL`. Current counts: 3,456 unreviewed, 418 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,456 unreviewed, 491 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 target-move XACT source pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_59_76`, SPX v4 `PROCEED_XACT_59_76`, and SPX v5 `DM2_PROCEED_XACT_59_76`. Firestaff models item selection and handling checks, but the source paths also issue 19f0_2165 toward the live target; Firestaff's runtime XACT helper fails closed before that command and has no production caller. All three rows remain `PARTIAL`. Current counts: 3,453 unreviewed, 421 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,453 unreviewed, 494 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 69 source correction (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_69`, SPX v4 `PROCEED_XACT_69`, and SPX v5 `DM2_PROCEED_XACT_69`. All set a facing-adjacent target tile and update live creature target/action fields; this operation does not emit sound. Firestaff's helper computes related coordinates/command bytes but is mislabeled as sound and does not mutate live state or bind production callers. All three rows remain `PARTIAL`. Current counts: 3,450 unreviewed, 424 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,450 unreviewed, 497 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 64 source pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_64`, SPX v4 `PROCEED_XACT_64`, and SPX v5 `DM2_PROCEED_XACT_64`. Firestaff mirrors possession, flag, and item checks, but the source paths also issue projectile command 0x81 and restore live flags; Firestaff's handler fails closed before launch and is not production-bound. All three definitions remain `PARTIAL`. Current counts: 3,447 unreviewed, 427 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,447 unreviewed, 500 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 70 source pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_70`, SPX v4 `PROCEED_XACT_70`, and SPX v5 `DM2_PROCEED_XACT_70`. Sources construct target/action fields from party position and facing, check the creature at the adjacent target, and conditionally test item handling. Firestaff has a source-mapped receipt, but its runtime helper fails closed before lookup/state mutation and has no production caller. All three definitions remain `PARTIAL`. Current counts: 3,444 unreviewed, 430 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,444 unreviewed, 503 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 63 source correction (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_63`, SPX v4 `PROCEED_XACT_63`, and SPX v5 `DM2_PROCEED_XACT_63`. These check whether a creature ahead can handle the requested item; they do not cast spells. Firestaff has a matching query helper, but its XACT dispatcher does not call it in production and labels the operation as cast spell. All three rows remain `PARTIAL`. Current counts: 3,441 unreviewed, 433 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,441 unreviewed, 506 partial source/older-ledger rows, 2 partial source owners).


## Latest source review: equipment assessment callback (2026-11-01)

DOS `SKW_14cd_2807`, SPX v4 `_14cd_2807_CREATURE`, and SPX v5 `DM2_14cd_2807` all score usable possessed items through the 48ae query; their paired `14cd_2886` routine walks the possession chain. Firestaff has callback-shaped scoring and chain-walk helpers, but no production combat/AI binding. All three remain `PARTIAL`. Active-source counts: 3,438 unreviewed, 436 partial, 50 implemented-parity.


## Latest source review: XACT 68 combat assessment (2026-11-01)

DOS `SKW_PROCEED_XACT_68`, SPX v4 `PROCEED_XACT_68`, and SPX v5 `DM2_PROCEED_XACT_68` compare facing-creature attack/defense values using the 14cd_2886 and 48ae_0767 paths, then update live AI state. Firestaff has a corresponding receipt helper but no production dispatcher/state-commit binding. All three remain `PARTIAL`. Active-source counts: 3,435 unreviewed, 439 partial, 50 implemented-parity.


## Latest source review: XACT 77 walk-path planning (2026-11-01)

DOS `SKW_PROCEED_XACT_77`, SPX v4 `PROCEED_XACT_77`, and SPX v5 `DM2_PROCEED_XACT_77` walk type-17 plan entries, append walk candidates, and search for a path. Firestaff has a structure-only receipt without live-list traversal or production path binding. All three remain `PARTIAL`. Active-source counts: 3,432 unreviewed, 442 partial, 50 implemented-parity.


## Latest source review: XACT 78 map movement (2026-11-01)

DOS `SKW_PROCEED_XACT_78`, SPX v4 `PROCEED_XACT_78`, and SPX v5 `DM2_PROCEED_XACT_78` calculate direction toward the party, test adjacent-tile passability on the same map, and issue the movement turn. Firestaff has only the map-match receipt; the movement calculation/query/dispatch has no production binding. All three remain `PARTIAL`. Active-source counts: 3,429 unreviewed, 445 partial, 50 implemented-parity.


## Latest source review: XACT 80 movement delegation (2026-11-01)

DOS `SKW_PROCEED_XACT_80`, SPX v4 `PROCEED_XACT_80`, and SPX v5 `DM2_PROCEED_XACT_80` set temporary movement flags, derive the direction/mode, delegate to `CREATURE_GO_THERE`, then restore the saved flags. Firestaff records the intended fields without live movement or production binding. All three remain `PARTIAL`. Active-source counts: 3,426 unreviewed, 448 partial, 50 implemented-parity.


## Latest source review: XACT 83 command selection (2026-11-01)

DOS `SKW_PROCEED_XACT_83`, SPX v4 `PROCEED_XACT_83`, and SPX v5 `DM2_PROCEED_XACT_83` check bit 7/selector, write a bounded `0x23`-series command, and return variant result codes. Firestaff models the decision in a receipt but does not apply it to live data through a production caller. All three remain `PARTIAL`. Active-source counts: 3,423 unreviewed, 451 partial, 50 implemented-parity.


## Latest source review: XACT 91 possession item checks (2026-11-01)

DOS `SKW_PROCEED_XACT_91`, SPX v4 `PROCEED_XACT_91`, and SPX v5 `DM2_PROCEED_XACT_91` query two selected item ids against the creature possession and pass if either is usable. Firestaff models the result in helpers but has no production XACT binding. All three remain `PARTIAL`. Active-source counts: 3,420 unreviewed, 454 partial, 50 implemented-parity.


## Latest source review: creature walk dispatcher (2026-11-01)

DOS `SKW_CREATURE_WALK_NOW`, SPX v4 `CREATURE_WALK_NOW`, and SPX v5 `DM2_CREATURE_WALK_NOW` coordinate attack setup, movement planning, pit/teleport handling, record movement and recovery. Firestaff has a similar callback helper but no production DM2 runtime caller or fully verified platform callback ownership. All three remain `PARTIAL`. Active-source counts: 3,417 unreviewed, 457 partial, 50 implemented-parity.


## Latest source review: CCM06 direction update (2026-11-01)

DOS `SKW_1887_05aa`, SPX v4 `CREATURE_CCM06`, and SPX v5 `DM2_CREATURE_CCM06` select and store a new direction when facing opposite the target. Firestaff has the matching helper and a production CCM route, but source RNG sequence and all-platform route coverage remain unverified. All three remain `PARTIAL`. Active-source counts: 3,414 unreviewed, 460 partial, 50 implemented-parity.


## Latest source review: CCM0B path/message action (2026-11-01)

DOS `SKW_1887_0c75`, SPX v4 `CREATURE_CCM0B`, and SPX v5 `DM2_CREATURE_CCM0B` invoke the 19f0_0d10 path/message condition, then issue target-tile message type 2 for action 0x0b. Firestaff has a callback implementation but lacks verified production route and full platform message ownership. All three remain `PARTIAL`. Active-source counts: 3,411 unreviewed, 463 partial, 50 implemented-parity.


## Latest source review: creature shoot-item action (2026-11-01)

DOS `SKW_1887_0eca` and SPX v5 `DM2_CREATURE_SHOOT_ITEM` select/cut a usable possession item, derive randomized projectile damage, clamp it, and launch it. The v5 row was already partial; DOS is now recorded as partial because Firestaff does not bind the complete selection-to-launch sequence in production. Active-source counts: 3,410 unreviewed, 464 partial, 50 implemented-parity.


## Latest source review: creature transformation (2026-11-01)

DOS `SKW_1887_126d`, SPX v4 `CREATURE_TRANSFORM`, and SPX v5 `DM2_CREATURE_TRANSFORM` perform transformation effects, replacement creature/AI/HP/animation setup, level checks, and possible record deletion. No corresponding Firestaff implementation was found. All three are `MISSING`. Active-source counts: 3,407 unreviewed, 464 partial, 3 missing, 50 implemented-parity.


## Latest source review: explode/summon creature action (2026-11-01)

DOS `SKW_1887_141c`, SPX v4 `CREATURE_EXPLODE_OR_SUMMON`, and SPX v5 `DM2_CREATURE_EXPLODE_OR_SUMMON` implement explosion, minion creation or transformation/rebind modes. Firestaff routes a related callback through CCM, but exact platform-specific state, RNG, and side effects remain partial. All three remain `PARTIAL`. Active-source counts: 3,404 unreviewed, 467 partial, 3 missing, 50 implemented-parity.


## Latest source review: 1BA1B tile passability predicate (2026-11-01)

DOS `R_1BA1B` and SPX v5 `DM2_1BA1B` decide path/LOS admission for floor, open tiles, teleporters and rebirth-altar doors. Firestaff has the predicate helpers, but the production path/LOS callbacks and supported-platform binding remain open. Both remain `PARTIAL`. Active-source counts: 3,402 unreviewed, 469 partial, 3 missing, 50 implemented-parity.


## Latest source review: 19f0_045a tile-cache refresh (2026-11-01)

DOS `SKW_19f0_045a`, SPX v4 `_19f0_045a`, and SPX v5 `DM2_19f0_045a` refresh cached tile and record-search fields when map/coordinates change. Firestaff has the cache helper, but production map/cache caller binding across platforms is unverified. All three remain `PARTIAL`. Active-source counts: 3,399 unreviewed, 472 partial, 3 missing, 50 implemented-parity.


## Latest source review: 19f0_04bf record-chain walk (2026-11-01)

DOS `SKW_19f0_04bf`, SPX v4 `_19f0_04bf`, and SPX v5 `DM2_19f0_04bf` return the cached first eligible tile record or skip record types 0-3 and cache the result. Firestaff has the cache helper, but production record-chain caller binding remains unverified. All three remain `PARTIAL`. Active-source counts: 3,396 unreviewed, 475 partial, 3 missing, 50 implemented-parity.


## Latest source review: 19f0_050f creature-record cache scan (2026-11-01)

DOS `SKW_19f0_050f`, SPX v4 `_19f0_050f`, and SPX v5 `DM2_19f0_050f` scan cached tile records to the first DB4 creature or end marker. Firestaff has the cache helper, but production map-record caller binding remains unverified. All three remain `PARTIAL`. Active-source counts: 3,393 unreviewed, 478 partial, 3 missing, 50 implemented-parity.


## Latest source review: 1c9a_0247 cache deallocation (2026-11-01)

DOS `SKW_1c9a_0247`, SPX v4 `_1c9a_0247_FREE_CACHE_FROM_CREATURE`, and SPX v5 `DM2_1c9a_0247` free two allocator/cache keys. V4 uses a different second selector (0x28000000); Firestaff’s wrapper remains a no-op, lacks that v4 variant, and has no production allocator/free binding. All three remain `PARTIAL`. Active-source counts: 3,390 unreviewed, 481 partial, 3 missing, 50 implemented-parity.


## Latest source review: GET_CREATURE_AT record-chain lookup (2026-11-01)

DOS `SKW_GET_CREATURE_AT`, SPX v4 `GET_CREATURE_AT`, and SPX v5 `DM2_GET_CREATURE_AT` find the first DB4 creature in a tile chain or return the null sentinel. Firestaff’s shared helper has the same filter and live AI/timer/world callers; corrupt chains fail closed. All three are `IMPLEMENTED_PARITY`. Active-source counts: 3,387 unreviewed, 481 partial, 3 missing, 53 implemented-parity.
