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
definitions from `SKWINSPX/src`, `SKWINDOS/src`, and the legacy DOS files at
the top level of `SKWINDOS/` in the locked checkout: 3,929 definition rows,
3,629 distinct names, and 300 repeated-name definitions. The active review
ledger currently maps 3,860 as `UNREVIEWED`, 64 as `PARTIAL`, and 5 as
`IMPLEMENTED_PARITY`; exact name overlap with the older ledger is only a
search hint, not proof of semantic parity. Editor and asset-tool trees (`DM2GDED`,
`DMDC2`, and `Utils`) are excluded because they do not implement the game
runtime.

## Inventory

| Source set | Definitions | Result |
|---|---:|---|
| Project-owned `.cpp` in `SKWIN/` and `SKULLWIN/`, plus selected `DM2_ARRANGE_DUNGEON` | 1,754 | Callable-definition rows in the disposition ledger |
| `SKWINSPX/src`, `SKWINDOS/src`, and legacy top-level `SKWINDOS/` | 3,929 (2,724 SPX, 1,205 DOS) | 3,860 `UNREVIEWED`; 64 `PARTIAL`; 5 `IMPLEMENTED_PARITY` |

## Dispositions

| Status | Count |
|---|---:|
| `IMPLEMENTED_PARITY` | 1,091 |
| `NOT_APPLICABLE_ARCH` | 560 |
| `NONAPPLICABLE` | 73 |
| `UNOWNED_SOURCE_MUTATION` | 0 |
| `PARTIAL` | 28 |
| `STATIC_INPUT_ONLY` | 0 |
| `PARTIAL_SOURCE_OWNER` | 2 |
| `MISSING` | 0 |
| **Total** | **1,754** |

The supplemental inventory covers 17 SPX v5 and 15 DOS `c_alloc.cpp`
definitions, SPX v4 `ACTUATE_DOOR` and `STEP_DOOR`, and legacy top-level DOS source. Reviewed entries are recorded in the companion source-review ledger with source mappings and variant-specific evidence. The DOS
`DM2_ALLOC_MEMORY_RAM` body also ignores the clean flag and always clears
the allocated range, unlike the SPX branch. These source variants still
lack a live Firestaff allocator owner.

The legacy DOS shooter definitions in `SKWINDOS/dm2byg.cpp` and `dm2byc.cpp` are also recorded as partial in the supplemental review; their implementations do not have a complete live projectile owner. The DOS `SKW_IS_REBIRTH_ALTAR` predicate is also mapped to Firestaff’s matching helper; map-header input decoding and caller parity remain open for each supported platform. `SKW_QUERY_DOOR_STRENGTH` is source-compared to the callback helper. SPX v4 and v5 door-query variants are now separately reviewed. DOS/v5 strength reads field 0x11 with the field-0x10 fallback; SPX v4 reads field 0x0f directly. SPX v4 resistance reads field 0x0e and gets graphics from map-header flags/door types, while DOS/v5 use GDAT fields 0x0f/0x0d. Firestaff's map-door metadata path already selects parsed door sets behind header enable flags and door-record type; exact 0xff no-door consumer behavior and complete caller coverage remain open. Four Firestaff query receipts were corrected to match the locked skproject field indices; per-platform asset and runtime verification remain open.

The SKULLWIN shooter audit also follows live callers: `DM2_ACTIVATE_SHOOTER` reaches a fail-closed helper before projectile creation, while `DM2_SHOOT_ITEM` and `DM2_CREATURE_SHOOT_ITEM` helpers have no live callers. Their source projectile allocation, initialization, timer, and creature lifecycle behavior is therefore `PARTIAL`, not parity-complete. Matching SKWINSPX v4/v5 source definitions are recorded in the supplemental review ledger. The matching DOS implementations live in legacy top-level `SKWINDOS/` files and are now included in the expanded supplemental inventory; only individually mapped entries are reviewed.

The older allocator rows were rechecked against the current Firestaff call graph. The 16 `SKULLWIN/c_alloc.cpp` entries previously marked `IMPLEMENTED_PARITY` now remain `PARTIAL`: their mapped `dm2_v1_skproject_*_classify` routines produce classifier receipts or mutate a model, but no live Firestaff allocator owner calls them. This does not prove the source free-pool allocation, pointer movement, setup/teardown, and runtime behavior. The analogous `SKWINSPX` and `SKWINDOS` definitions remain individually unreviewed in the supplemental inventory.

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
claims that a supported DM2 game feature is implemented. The 1,754 disposition
rows cover the older `SKWIN`/`SKULLWIN` source sets; the expanded supplemental
inventory contains 3,929 `SKWINSPX`/`SKWINDOS` definitions, with 3,870 still
unreviewed and individual partial or parity dispositions recorded separately. Per-function comparison and runtime evidence remain required for
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

At this revision it reports 3,953 open rows: 3,860 unreviewed active-source
definitions, 91 partial source or older-ledger rows, and 2 partial source-owner
rows. The active inventory now includes the legacy DOS implementation files. Exact-name overlap is shown only as a
candidate for source comparison.

```sh
awk -F '\t' 'NR > 1 { count[$6]++; rows++ }
  END { print rows, count["IMPLEMENTED_PARITY"], count["NOT_APPLICABLE_ARCH"],
              count["NONAPPLICABLE"], count["UNOWNED_SOURCE_MUTATION"],
              count["PARTIAL"], count["STATIC_INPUT_ONLY"],
              count["PARTIAL_SOURCE_OWNER"] }' \
  docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv
```
