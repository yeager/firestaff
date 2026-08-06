# Chaos Strikes Back — Reverse Engineering Notes

This page consolidates the reverse-engineering findings behind Firestaff's
CSB implementation: source module inventory, the Amiga IMG1 graphics
container, the CSBWin DSA (Dynamic Scripting Architecture) system, the CSB
save format, the viewport rendering path, and the reference boundary between
ReDMCSB and CSBWin. For narrative status/progress see
[CSB-Technical-Reference](CSB-Technical-Reference) and for the authenticated
DSA/save contract see [CSB-DSA-and-Save-Internals](CSB-DSA-and-Save-Internals).

## 1. CSB-Specific Modules

Firestaff's CSB implementation lives in `src/csb/` (357 `.c` files) and
`include/csb_v1_*` / related headers (245+ files), plus `redmcsb_*` modules
that port ReDMCSB functions unmodified for CSB reuse.

```bash
ls src/csb/*.c | sort      # 357 files
ls include/csb_v1_* | sort # 245 files
```

### Module families (by prefix)

| Prefix | Purpose | Example files |
|---|---|---|
| `csb_v1_fNNNN_*` | Direct ReDMCSB function ports (F0089-F2605), one module per function or function range | `csb_v1_f0267_move_result_pc34_compat.c`, `csb_v1_f0247_launcher_materialization_pc34_compat.c` |
| `redmcsb_fNNNN_*` | ReDMCSB functions shared verbatim between DM1 and CSB (text, video primitives, IMG3, save headers) | `redmcsb_f0684_blit_pc34_compat.c`, `redmcsb_f7061_save_header_pc34_compat.c` |
| `csb_v1_viewport_*` | Per-element/per-depth viewport decomposition (D0-D3 walls, doors, ornaments, projectiles, custom backgrounds) | `csb_v1_viewport_d2l_d2r_wall_pc34_compat.c`, `csb_v1_viewport_d1c_f0095_floor_ornament_pc34_compat.c` |
| `csb_v1_csbgraphics_*` / `csb_v1_csbwin_*` | CSBWin-specific extensions: custom `CSBgraphics.dat`, XOR-padded saves, DSA runtime admission, save-loader boundary | `csb_v1_csbwin_dsa_runtime_admission_pc34_compat.c`, `csb_v1_csbwin_save_loader_boundary_pc34_compat.c` |
| `csb_v1_atari_*` / `csb_v1_fmtowns_*` / `csb_v1_amiga_*` / `csb_v1_amg_*` | Non-DOS platform ports (Atari ST saves/animation, FM Towns CD/ANM/portrait, Amiga IMG1 graphics, Amiga sound) | `csb_v1_atari_save_decode_pc34_compat.c`, `csb_v1_fmtowns_cd.c` |
| `csb_v1_startup_*` | Presentation chain: C001-C005 raster phases, SWSH, entrance, credits | `csb_v1_startup_raster_present_pc34_compat.c`, `csb_v1_f0806_entrance_loop_runtime_handoff_pc34_compat.c` |
| `csb_v1_save_*` / `csb_v1_utility_*` | Original save read/write, export/import, Utility Disk transaction path | `csb_v1_save_export_import_pc34_compat.c`, `csb_v1_utility_save_transaction_pc34_compat.c` |
| `csb_v2_*` / `csb_v22_*` | Modern presentation layer: filters, HUD overlay, minimap, dynamic lighting, VFX, touch controls (non-parity, opt-in) | `csb_v2_filter_crt_scanlines_pc34.c`, `csb_v22_inplace_draw_pc34.c` |
| `csb_hint_oracle_*` / `csb_p4_*` | Hint system (HTC files) and phase-4 lighting/VFX metadata | `csb_hint_oracle_htc.c`, `csb_p4_lighting_metadata.c` |

## 2. IMG1 Graphics Format (Amiga)

CSB's original PC data file is `CSBGRAPHICS.DAT`, but it is **not** encoded
like DM1's PC `GRAPHICS.DAT`. It uses the Amiga v3.1 **IMG1 nibble-RLE**
format inherited from the CSB Amiga release, wrapped in a big-endian
`DMCSB2` container.

### Container header

| Field | Encoding | Notes |
|---|---|---|
| Marker | `0x8001` (BE) | `CSB_AMIGA_GRAPHICS_CONTAINER_WORD` |
| Item count | `uint16` BE | Known variants: 749 items (Amiga 3.1/3.3 ML) |
| Size bounds | 300,000–500,000 bytes | `CSB_AMIGA_GRAPHICS_MIN_SIZE` / `MAX_SIZE` gate |
| Per-item header | BE width/height | precedes IMG1 nibble-RLE payload |

Known hash-identified variants (`include/csb_v1_amiga_graphics_dat.h`):

| MD5 | Variant | Item count |
|---|---|---|
| `61fbfd56887c94adc26888a9491c6611` | Amiga 3.1/3.3 Multilanguage | 749 |
| `291e1bc6803e3dc4b974c60117ca5d68` | Amiga 3.5 English | — |
| `cefaddfdf5651df2c91f61b5611a8362` | Amiga 3.5 Multilanguage | — |
| `21197b1d4994fd835c403d5a33dcac2b` | Amiga X.X/3.1 English | — |

### Nibble-RLE encoding

Each IMG1 graphic is decoded via a 4-bit (nibble) run-length scheme, distinct
from DM1's byte-oriented IMG3 compressed-line format. Firestaff replaced its
`ExpandGraphic` byte-format decoder with a dedicated IMG1 nibble-RLE decoder
(`csb_v1_amiga_graphics_dat.c`, `csb_v1_graphics_lzw_pc34_compat.c` family)
because reusing the PC IMG3 path silently mis-decoded CSB Amiga-sourced art.

Two title/entrance graphics anchor correctness:

| Graphic | Size | Role |
|---|---|---|
| C001 | 320×153 | Title screen ("CHAOS STRIKES BACK") |
| C004 | 320×200 | Dungeon entrance backdrop |

Both decode 100% correctly through the IMG1 path per
`CSB-Technical-Reference.md`.

### IMG1 vs IMG3 — key differences

| Aspect | DM1 IMG3 (PC) | CSB IMG1 (Amiga-derived) |
|---|---|---|
| Byte order | Little-endian | Big-endian (`DMCSB2` container) |
| Unit of RLE run | Byte | Nibble (4-bit) |
| Container | Flat PC `GRAPHICS.DAT` offsets | `0x8001` marker + BE item count wrapper |
| Reference decoder | ReDMCSB `IMAGE3.C` / `F0689_Img3_Expand` | ReDMCSB `IMAGE1.C` nibble-RLE notes + Amiga corpus |
| Firestaff module | `csb_v1_img3_asset_presentation_pc34_compat.c` (PC-path CSB fallback) | `csb_v1_amiga_graphics_dat.c` (primary CSB path) |

CSBWin's own `CSBgraphics.dat` companion format (produced by CSBGraffer /
CSBWin Viewport Compiler) is a **third**, separate container — classified by
`csb_v1_csbgraphics_dat_classify.c` and bound at runtime via
`csb_v1_csbgraphics_runtime_binding.c` / `csb_v1_csbgraphics_runtime_plan.c`.
It must not be confused with either the PC or Amiga `CSBGRAPHICS.DAT`.

## 3. DSA (Dynamic Scripting Architecture)

CSBWin's DSA system is a stack-based scripting VM used by custom dungeons
(and embedded in the original CSB dungeon for some triggers). It is unique
to the CSBWin PC port — ReDMCSB does not define it. Firestaff treats a
loaded DSA action as **authenticated source data**, never a general-purpose
scripting API: unsupported opcodes and malformed records fail closed rather
than falling back to a generic interpreter.

| Property | Value |
|---|---|
| Unique opcodes covered | 117 |
| Test coverage | 12 files, 9255 lines (Q-CSB-01) |
| Reference header | CSBWin DSA runtime header, 998 lines, 264 CSBWin source references |
| Execution model | Scratch stack + candidate output state; publish only on complete action |

### Record identity

Every DSA record is identified by the source tuple:

```text
(absolute_dsa_id, state, ordinal, source_action_pointer)
```

The pointer identity is retained at runtime — a host-constructed or copied
action with matching words cannot be substituted into an authenticated
runner.

### Actuator binding (type-47 DSAselector)

A type-47 DB3 actuator does not name a DSA directly; its selector bits
resolve through the imported source level table:

```text
selector = (DB3.word2 >> 7) & 0x1f
dsa_id   = DSALevelIndex[current_level][selector]
```

Binding rejects on wrong actuator type, missing index, undefined ID, or an
action absent from the staged extension — never a synthesized default.

### Opcode families implemented

NOOP, EQUAL, QUESTION, plus STKOP families: `Loc2AbsCoord`,
`FetchExCellFlg`, `BitCount`, `ParamFetch`/`ParamStore`, `GlobalFetch`,
`PartyDistance`, `TimeFetch`, `ThisDSAId`, `WhoHasTalent`, `CountInjury`,
`TalentsFetch`, `DisableSaves`, `ChPoss`/`MonPoss`, `ExamineCell`, `Copy`,
`CharFetch`/`CharStore`, `SwapCharacter`, `CausePoison`, `Mastery`,
`MissileInfoFetch`/`Store`, `MonsterFetch`, `PartyFetch`, `Override`,
`Message`, `Overlay`, `Palette`, `ExperiencePlus`, `JumpGear`/`GosubGear`.

`CausePoison` and `CountInjury` double as combat integration points (Q-CSB-08
damage-character filter), not pure stack primitives.

### LOAD/STORE, banks, and control transfer

- **LOAD/STORE**: source-shaped, operating on local `DSAVARS` and owned
  global banks. Out-of-bank variables/globals reject.
- **JUMP/GOSUB**: follow CSBWin `Execute` selection rules —
  - state/column lookup picks the **first exact file-order** `(state,
    column)` action match;
  - `JUMP` transfers only within the bounded execution frame;
  - `GOSUB` records a one-frame nested transfer and preserves outer
    continuation;
  - a missing target ends selection without inventing a synthetic action;
  - depth and transfer ceilings reject the action before publication.
- World-mutating AMPERSAND forms, unsupported loads, malformed words, and
  unowned pointers all reject — a rejection leaves parameters, globals,
  filter receipt, and dungeon state unchanged (transactional publish).

### Module map

| Concern | Module |
|---|---|
| DSA runtime admission gate | `csb_v1_csbwin_dsa_runtime_admission_pc34_compat.c` |
| Movement/multilevel/timer filters, trigger single-step | covered by `test_csb_v1_dsa_trigger_single_step_pc34_compat`, `test_csb_v1_f0267_loaded_chain_pc34_compat` |
| Combat opcodes | `csb_v1_grey_lord_combat_pc34_compat.c`, damage-character filter |
| 512-byte XOR pad classification (extended saves) | `csb_v1_csbwin_512_xor_pad_classify.c` |

Full opcode-by-opcode detail, admitted/restored timer bridge semantics, and
the real-package DSA receipt contract (`firestaff_csb_v1_csbwin_extended_dsa_handoff_probe`)
are documented in [CSB-DSA-and-Save-Internals](CSB-DSA-and-Save-Internals).

## 4. CSB Save Format

CSB save handling spans two layers: the original ReDMCSB save/dungeon-tail
contract (shared with DM1) and CSBWin's extended-save format for DSA/custom
dungeons.

### Original save (ReDMCSB-derived)

| Concern | Module |
|---|---|
| Save header build/read | `redmcsb_f7061_save_header_pc34_compat.c`, `redmcsb_f7062_save_header_pc34_compat.c` |
| Dungeon-part checksum | `redmcsb_f7059_dungeon_part_checksum_pc34_compat.c` |
| Dungeon stream | `redmcsb_f7063_dungeon_stream_pc34_compat.c` |
| Champion text / portraits | `redmcsb_f7064_champion_text_pc34_compat.c`, `redmcsb_f7065_portrait_slots_pc34_compat.c`, `redmcsb_f7067_portrait_info_pc34_compat.c`, `redmcsb_f7088_portrait_transfer_pc34_compat.c` |
| New-adventure creation | `redmcsb_f7090_make_new_adventure_pc34_compat.c` |
| Dungeon/save tail | `redmcsb_f0434_dungeon_tail_pc34_compat.c`, `redmcsb_f0435_save_tail_pc34_compat.c` |

### GAMEBLOCK1/body import and extended tails

CSBWin extends the original save with GAMEBLOCK2, CHARDESC, ITEM16, timers,
game-info, DSA action records, level-index data, and an optional trace
bitmap. The extension loader **stages** all of these before a live profile
is changed — failure in a later record invalidates the whole candidate
rather than keeping an earlier partial import (`csb_v1_csbwin_save_loader_boundary_pc34_compat.c`).

- The CSBWin save loader boundary rejects malformed **non-empty** DB11/EXPOOL
  tails before atomic runtime staging.
- `csb_v1_csbwin_dungeon_tail.c` handles the CSBWin-specific dungeon tail
  layout distinct from the ReDMCSB `F0434` tail.

### F0435 native provenance

`F0435` provenance is recorded **only after a committed import**, never on a
rejected candidate — this is the "did this save actually come from a real
CSB/CSBWin file" marker used elsewhere in the runtime to gate CSBWin-only
behavior (DSA execution, extended timers).

### Export/import and Utility Disk

| Concern | Module |
|---|---|
| Export/import round trip | `csb_v1_save_export_import_pc34_compat.c` |
| Save import path | `csb_v1_save_import_path_pc34_compat.c` |
| Load | `csb_v1_save_load_pc34_compat.c` |
| Real-artifact boundary (hash-gated) | `csb_v1_save_real_artifact_boundary_pc34_compat.c` |
| Utility Disk transaction (import/edit/inventory/dialogs) | `csb_v1_utility_save_transaction_pc34_compat.c`, `csb_v1_utility_flow_pc34.c`, `csb_v1_utility_import_pc34.c` |

Test coverage: 32 save/Utility Disk files (Q-CSB-09), 15 of which are
executable test binaries; all pass.

### Timer queue restart boundary

For a resumed CSBWin save, `TIMER`/`TimerQueue` state remains source-owned:
core export retains original array indexes, heap topology, sequence words,
and GAMEBLOCK2 timer counters **only** when every live timeline entry still
has an exact saved queue-slot receipt (time, function, priority,
coordinates, cell, effect). A fired, replaced, duplicated, or unmapped event
breaks the receipt and CSBWin export rejects rather than emit a
reconstructed queue.

## 5. Viewport and Rendering

### CSB path

```
csb_v1_viewport_render_frame()          [src/csb/csb_v1_viewport_pc34_compat.c]
        │  builds DM1_Viewport3DState from CSB_V1_ViewportConfig
        ▼
dm1_viewport_3d_draw_frame()            [shared DM1 V1 viewport engine]
```

`csb_v1_viewport_render_frame(CSB_V1_ViewportConfig *cfg, int party_dir, int
party_x, int party_y)` is the integration entry point (called from
`csb_v1_boot.c`). It is a documented passthrough to the shared DM1 V1
viewport engine (source: CSBWin `Viewport.cpp` F0128; ReDMCSB `DUNVIEW.C`
F0128), reusing the same 320×200 indexed pixel format and 224×136 viewport
sub-region at screen row 33 so DM1 draw primitives work unmodified. It is a
no-op when `viewport_pixels` is `NULL` (staged-integration guard).

CSB config supplies:
- `viewport_pixels` + `stride` — target pixel buffer
- `dungeon_grid`/`width`/`height` — square-type data for element routing
- `wall_set_index` — selects which `GRAPHICS.DAT`/`CSBGRAPHICS.DAT` wall set

### Callback-based wall ornament resolution

Wall ornament ordinals are resolved through
`csb_v1_viewport_wall_ornament_ordinal_resolver_pc34_compat.c`, which feeds
the shared `DM1_ViewportWallOrnamentOrdinalCallback` used by
`dm1_viewport_3d_draw_frame` — the same callback contract DM1's CSB-style
path (as opposed to the M11 monolithic path) uses.

### Per-depth/per-element decomposition

Unlike DM1's M11 path (a single large `m11_draw_viewport` with inline
dungeon-data access), CSB's viewport is decomposed into dozens of focused
modules, one per depth (D0-D3) × side (center/left/right) × element type:

| Depth/element | Example modules |
|---|---|
| Walls | `csb_v1_viewport_d1l_d1r_wall_pc34_compat.c`, `csb_v1_viewport_d2l2_d2r2_wall_pc34_compat.c`, `csb_v1_viewport_d3l2_wall_pc34_compat.c` |
| Doors (incl. partly-open) | `csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_pc34_compat.c`, `csb_v1_viewport_d3l2_door_pc34_compat.c` |
| Floor/ceiling ornaments | `csb_v1_viewport_d1c_f0095_floor_ornament_pc34_compat.c`, `csb_v1_viewport_d1c_f0108_floor_ceiling_ornament_pc34_compat.c` |
| Center fields | `csb_v1_viewport_d0c_center_field_pc34_compat.c` … `csb_v1_viewport_d3c_center_field_pc34_compat.c` |
| Custom backgrounds (11 variants) | `csb_v1_viewport_custom_backgrounds_room_slot_pc34_compat.c`, `csb_v1_viewport_custom_background_mask_apply_pc34_compat.c` |
| Things/projectiles | `csb_v1_viewport_d1c_f0115_thing_pass_pc34_compat.c`, `csb_v1_viewport_f0115_projectile_metadata_pc34_compat.c`, `csb_v1_viewport_d2l2_f0115_projectile_pc34_compat.c` |

F0115 first-object native graphics use the G0209 weapon[46]/armour[58]/
junk[52]/potion[21] tables with C10 blit (conditional horizontal flip).
Creature groups use per-creature transparency (G0219
`coordinateSet_transparentColor`) and D2/D3 palette remap tables
(G0221/G0222).

Test coverage: 47 viewport tests (Q-CSB-06) covering walls D0-D3 all sides,
doors, partly-open doors, ornaments, floor/ceiling, backgrounds, center
fields, footprints, and projectile routing.

### CSB vs DM1 M11 path

| Aspect | DM1 M11 path | CSB path |
|---|---|---|
| Entry point | `m11_draw_viewport` (`m11_game_view.c`, ~50k-line file) | `csb_v1_viewport_render_frame` → `dm1_viewport_3d_draw_frame` |
| Data access | Inline dungeon-data access inside the monolithic function | Routed through `CSB_V1_ViewportConfig` + shared DM1 V1 engine |
| Wall ornament source | Direct resolution inside M11 | Callback (`DM1_ViewportWallOrnamentOrdinalCallback`) via dedicated resolver module |
| Decomposition | Single large function | ~50+ per-depth/per-element modules |
| Graphics container | PC IMG3 `GRAPHICS.DAT` | IMG1 (Amiga) or optional CSBWin `CSBgraphics.dat` |

## 6. ReDMCSB vs CSBWin Reference Boundary

ReDMCSB and CSBWin answer **different** questions and are not
interchangeable evidence.

| Area | Primary evidence | What the other reference cannot prove |
|---|---|---|
| Original timers and dungeon mutation | ReDMCSB `TIMELINE.C`, `DUNGEON.C`, `GROUP.C` | CSBWin TIMER queue ownership and `timerObj6/8` semantics are CSBWin-only |
| CSBWin DSA/custom dungeons | CSBWin `DSA.cpp`, `data.cpp`, `SaveGame.cpp` | ReDMCSB has no EXPOOL, type-47 selector tables, opcode behavior, or DSA state — the DSA system does not exist in the original engine |
| Save interoperability | ReDMCSB `DEFS.H`, `LOADSAVE.C` + per-media corpus | ReDMCSB cannot prove GAMEBLOCK2, ITEM16, extended tails, EXPOOL, or CSBWin continuation bytes |
| Mouse, audio, interrupt timing | target-media capture; CSBWin host sources for CSBWin paths | ReDMCSB's vector-dispatched `USIOSTUB.C`/`MUSCSTUB.C`/interrupt timing establishes a dispatch path exists, not portable/SDL-equivalent behavior |
| Title/entrance/HUD pixels | hash-identified original assets + capture | Neither reference alone defines a universal palette, cadence, or renderer across `MEDIA*` branches |

ReDMCSB is the primary source for original CSB engine behavior, original
EVENT/timeline structure, media branches, and original save-header
contracts — it is explicitly **not** evidence for CSBWin extensions.
CSBWin is the primary (and only) source for DSA, extended saves, and
CSBWin-specific champion/input behavior.

The ReDMCSB archive itself contains platform dispatch and assembly segments
rather than a universal host spec: `USIOSTUB.C` forwards mouse/queued-input
through library vectors, `MUSCSTUB.C` forwards music calls, `VBLANK.C`
installs interrupt handlers, `GRAPH21.C` carries media-specific CPSE/fuzzy-
sector logic. These establish that a code path exists on the original
platform, not that Firestaff's SDL3 port reproduces equivalent ordering,
sampling, mixing, or copy-protection results.

### Required evidence tuples

For any claimed original save format:

```text
(media/version hash, original save bytes, parser receipt, runtime capture)
```

For any CSBWin DSA/save claim:

```text
(CSBWin save hash, timer queue slot, source timer index,
 DSA/EXPOOL record identity, selected source action)
```

Absent these tuples, Firestaff fails closed or states the route is not yet
verified. The detailed gap-tracking queue is `REDMCSB-CSB-GAP-001` through
`REDMCSB-CSB-GAP-013` in `TODO.md`.

## 7. Parity Evidence

`parity-evidence/` contains the cross-game pass corpus. The exact current
count is repository-derived rather than maintained as a narrative number. 28
are
CSB-specific (`pass*csb*` or explicit CSB-scope filenames), covering DSA
opcode receipts, save loader boundaries, viewport element routing, IMG1
graphics decode, and the CSBWin extended-save handoff contract.

```bash
ls parity-evidence | grep -ci csb   # 28
ls parity-evidence | wc -l          # 1071
```

For the full pass index and evidence-document conventions, see
[Parity-Evidence](Parity-Evidence).

## See also

- [CSB-Technical-Reference](CSB-Technical-Reference) — Q-CSB-01 through
  Q-CSB-10 status and verification commands
- [CSB-DSA-and-Save-Internals](CSB-DSA-and-Save-Internals) — authenticated
  DSA/save/raster contract detail
- [DM1-PC34-Internals](DM1-PC34-Internals) — the shared ReDMCSB PC 3.4
  reference model that CSB inherits
- [Architecture-Overview](Architecture-Overview) — project-wide module map
