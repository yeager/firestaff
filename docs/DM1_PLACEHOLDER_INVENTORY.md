# DM1 placeholder and synthetic-data inventory (2026-08-06)

Systematic scan of every DM1 source path (`src/dm1/`, `include/dm1_*`,
`src/engine/*.c` uses of DM1 APIs) for placeholders, synthetic
substitutes, host-invented pixels/fonts/palettes/strings, and
capture-gated stubs. Scope excludes tests, contract-only receipts, and
cross-game code. Every remaining candidate is classified below; the
findings are then aggregated into the four current DM1 slices.

## Scan methodology

Five ripgrep passes over `src/dm1/`, `include/dm1_*.h`:

1. `TODO|FIXME|HACK` outside `_XXX_` constants (ReDMCSB naming).
2. `synthetic` outside `suppress|reject|refuse|guard|contract-only`.
3. `placeholder` any occurrence.
4. `host_(font|palette|colour|color|pixel|text|render|draw)`.
5. `fallback` outside `no fallback|refuse.*fallback|reject.*fallback|
   suppress.*fallback`.

Results triaged by opening every hit and classifying whether the code
is (a) a real host synthesis, (b) a receipt/policy guard describing
what is *not* allowed, (c) a ReDMCSB-named concept the original engine
itself owns (F0098 fallback, F0151 synthetic wall outside dungeon,
etc.), or (d) a diagnostic-only path already gated out of authenticated
runtimes.

## DM1 slice status

### 1. DM1 V1 — PC 3.4 parity lane (`src/dm1/*_pc34_compat.*`)

**Verdict: no open placeholders.** DONE.md line 558-564
(2026-08-06) records the full re-audit against the real
`GRAPHICS.DAT`/`DUNGEON.DAT` catalog and the `DM1-ORIGINAL-REPLACE-003
… -026` inventory: "source sessions either consume an authenticated
decoded surface or fail closed/no-draw."

Every candidate that survived the scan is one of:

- **Receipt/policy prose** — `suppressSyntheticFallback`,
  `suppressSyntheticRuntime`, `contract-only synthetic`, "no host
  polling or synthetic input" — describes what a receipt refuses to
  admit, not what the runtime emits. Examples:
  `dm1_v1_f0141_f0160_dungeon_source_receipt_pc34_compat.c:32`,
  `dm1_v1_f0201_f0220_action_source_receipt_pc34_compat.c:6`,
  `dm1_v1_f0341_f0360_render_action_source_receipt_pc34_compat.c:6`,
  `dm1_v1_f0355_inventory_material_pc34_compat.c:84`,
  `dm1_v1_f0662_invisibility_material_pc34_compat.c:78`,
  `dm1_v1_f0351_stats_material_pc34_compat.c:43`,
  `dm1_v1_f1006_f1025_source_ownership_pc34_compat.c:40`,
  `dm1_v1_f1126_f1145_source_ownership_pc34_compat.c:6`,
  `dm1_v1_f1386_f1405_local_ownership_pc34_compat.c:39`,
  `dm1_v1_action_spell_result_feedback_pc34_compat.c:82`,
  `dm1_v1_action_spell_render_consumption_pc34_compat.c`,
  `dm1_v1_action_spell_m11_host_render_pc34_compat.c` (M11 bridge
  that consumes an `original_valid_pc34` receipt and refuses
  otherwise).
- **ReDMCSB-owned concepts** — the original engine has a
  "synthetic wall just outside a dungeon" (F0151), an F0098
  floor/ceiling fallback, a C716/C717 wall-ornament overlay
  fallback, and a synthetic-first-room construction path. These are
  authentic ReDMCSB names; Firestaff mirrors them. Examples:
  `dm1_v1_dungeon_square_structs_pc34_compat.c:217`,
  `dm1_v1_viewport_f0098_floor_ceiling_fallback_pc34_compat.c`
  (whole module), the D0L2/D0R2/D2L/D2R/D3L2/D3R2 viewport modules,
  `dm1_v1_creature_ai_behavior_pc34_compat.c:1554`.
- **M11 host-render bridges** — files named `*_m11_host_render_*`
  are the framebuffer consumer that binds a source receipt to the
  M11 pixel buffer. They validate the receipt (e.g.
  `dm1_v1_action_spell_m11_host_render_original_valid_pc34`) and
  refuse to draw when it is missing. "host_render" here means
  "M11 host renderer for this feature", not "host-invented pixels".
- **Diagnostic-only overlay** — see combat log below.
- **Comment-only mentions** — `wound_probability_index_to_mask
  _pc34_compat.c:202` reserves a bit "as a placeholder for future
  use" (a bitfield reservation, not a rendering placeholder);
  `champion_panel_hand_slot_inventory_viewport_walk_pc34_compat.c
  :222` refers to the original game's own C201 empty-hand icon as
  its "placeholder" glyph (real game data).

### 2. DM1 V2.2 — modern-presentation lane (`include/dm1_v22_*`)

**Verdict: intentional placeholder art gated by an explicit
material-state gate.**

`include/dm1_v22_finished_art_material_gate_pc34.h` is a policy
module that classifies each modern-art slot into
`ART_STATE_{PLACEHOLDER, SYNTHETIC_TEST, REAL}` via a manifest with
required fields (`generator`, `source_file`, `resolution`). Slots
whose generator is `"placeholder"` are procedurally generated on
purpose — the V2.2 lane is a modernization target, not a parity
target, and the gate exists precisely so no V2.2 placeholder can
leak into a parity claim.

`include/dm1_v2_asset_pipeline_pc34.h:213` warns not to use
placeholder pixels when real data is available or expected — this is
the policy statement, not a violation.

**Action:** none required at the DM1 parity level. V2.2 modern-art
work is a separate deliverable; the gate correctly prevents its
placeholder art from being counted as parity.

### 3. DM1 FM Towns

**Verdict: no synthetic pixels; menu rendering blocked on decoded
EGB shim work.**

- Runtime cache materialization, CDDA end-to-end (title/HoC/entrance/
  map/event/toggle) and the FM Towns title compositor are wired
  against real hash-verified media, gated on
  `dm1FmtownsStartupReceiptValid`. See
  [DM1 FM Towns guide](wiki/DM1-FMTowns-Guide.md).
- The remaining `DRAW_DMENU`/`DYNAMENU` menu draw chain is decoded
  through `EGB_RECTANGLE`, `EGB_PUTBLOCK`, `EGB_COLOR`, `EGB_VIEWPORT`,
  `EGB_WRITEMODE`, `EGB_PAINTMODE`, `EGB_WRITEPAGE`,
  `EGB_RESOLUTIONRAM` at their exact executable offsets in
  [parity-evidence/dm1_fmtowns_menu_p3_disassembly.md](../parity-evidence/dm1_fmtowns_menu_p3_disassembly.md).
  The three bounded next steps (region-table lift, EGB shim,
  `DO_DRAW_CTEXT` decode) are real, non-synthetic work.
- Japanese `JDM.EXP` symbol table is not present in the executable;
  its title and menu addresses are not lifted. This blocks the
  Japanese runtime at the executable-title layer, not at a
  synthetic-fallback layer.

### 4. Combat-log diagnostic overlay (`dm1_v1_combat_log_pc34_compat.c`)

**Verdict: real host synthesis, but scope-gated to diagnostic worlds
only.**

The file defines a hardcoded 3x5 ASCII mini font at
`k_minifont_ascii32_126` (line 82) and a `draw_text_minifont` path
(line 180). `DM1_CombatLog_Render` at line 191 has two hard gates
before either path can run:

1. `DM1_CombatLog_SourceAllowsDiagnosticOverlay(state->sourceKind)`
   at line 209 — returns early for every authenticated PC34 source.
2. `DM1_CombatLog_SourceAllowsFallbackFont(state->sourceKind) &&
   !state->originalFontAvailable` at line 219 — the fallback font
   path is unreachable in an authenticated session even if the
   original font is missing (which would already be an asset
   failure).

Authenticated PC34 gameplay never sees a synthesised glyph from this
module. DONE.md line 539-543 (2026-08-06) records the same
conclusion: "Disabled the synthetic English combat-log overlay for
authenticated DM1 source sessions."

**Action:** none required. The synthesis is confined to the
diagnostic perimeter that the user asked to keep.

## What this inventory does *not* count

- **Cross-game modules** under `src/csb/`, `src/dm2/`, `src/nexus/`,
  `src/theron/`. DM2 has its own active DM2-only queue in TODO.md;
  Nexus, Theron and CSB have their own inventories in TODO.md.
- **Test fixtures** in `tests/` that construct synthetic worlds to
  exercise a specific code path. These are testing infrastructure.
- **Contract-only receipts** whose whole purpose is to declare "no
  host substitute is allowed"; they carry no pixels.

## Cross-references

- Prior DONE.md re-audit: DONE.md line 558-564 (2026-08-06).
- Diagnostic combat-log gate: DONE.md line 539-543.
- Q-DM1-01..Q-DM1-10 open work queue: `TODO.md` lines 993 onward
  for FM Towns items, `21521` onward for DM1 V1 follow-ups.
- FM Towns runtime guide: [wiki/DM1-FMTowns-Guide.md](wiki/DM1-FMTowns-Guide.md).
- FM Towns menu disassembly:
  [../parity-evidence/dm1_fmtowns_menu_p3_disassembly.md](../parity-evidence/dm1_fmtowns_menu_p3_disassembly.md).
